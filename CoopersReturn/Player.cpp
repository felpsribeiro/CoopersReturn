
#include "Player.h"
#include "Missile.h"
#include "CoopersReturn.h"

// -------------------------------------------------------------------------------
const Vector gravity = Vector(67.5f, 0.2f);

Player::Player()
{
    // configura��o do objeto
    sprite = new Sprite("Resources/Ranger.png");
    
    Point vertex[9] =
    {
        Point(-31, -9),
        Point(-24, -19),
        Point(1, -21),
        Point(28, -5),
        Point(28, 10),
        Point(1, 21),
        Point(-19, 15),
        Point(-28, 13),
        Point(-31, 2)
    };

    BBox(new Poly(vertex, 9));
    
    type = PLAYER;

    engine = new TileSet("Resources/Propellant.png", 50, 32, 1, 8);
    animEng = new Animation(engine, 0.120f, true);

    explosion = new TileSet("Resources/explosion.png", 240, 240, 8, 46);
    animExp = new Animation(explosion, 0.02f, false);

    speed = new Vector(0.0f, 0.0f);
    MoveTo(game->CenterX(), game->CenterY());
}

// -------------------------------------------------------------------------------

Player::~Player()
{
    delete sprite;
    delete speed;
    delete engine;
    delete animEng;
    delete animExp;
}

// -------------------------------------------------------------------------------

void Player::Move(Vector&& v, bool freio = false, bool shot = false)
{
    //if(!shot) v.Add(gravity);
    speed->Add(v);

    if(!freio) BBox()->RotateTo(360.0f - v.Angle());

    // limita velocidade m�xima
    if (speed->Magnitude() > 8.0f)
        speed->ScaleTo(8.0f);
}

// -------------------------------------------------------------------------------

void Player::Update()
{
    switch (CoopersReturn::state)
    {
    case INIT:
    case MESG:
    {
        animEng->NextFrame();
        break;
    }
    case EXPL:
    {
        animExp->NextFrame();
        if (animExp->Inactive())
            CoopersReturn::state = LOST;
        break;
    }
    case PLAY:
    case CLIMAX:
    case FINALIZE:
    {
        speed->Add(gravity);

        // magnitude do vetor acelera��o
        float accel = 30.0f * gameTime;

        if (CoopersReturn::ctrl) {
            CoopersReturn::gamepad->XboxUpdateState();

            float ang = Line::Angle(Point(0, 0), Point(CoopersReturn::gamepad->XboxAnalog(ThumbRX) / 25.0f, -1.0f * (CoopersReturn::gamepad->XboxAnalog(ThumbRY) / 25.0f)));
            float mag = Point::Distance(Point(0, 0), Point(CoopersReturn::gamepad->XboxAnalog(ThumbRX) / 25.0f, -1.0f * (CoopersReturn::gamepad->XboxAnalog(ThumbRY) / 25.0f)));

            // nenhuma direção selecionada
            if (mag == 0)
            {
                // se a velocidade estiver muita baixa
                if (speed->Magnitude() < 0.1)
                {
                    // apenas pare
                    speed->ScaleTo(0.0f);
                }
                else
                {
                    // comece a frear
                    Move(Vector(speed->Angle() + 180.0f, 5.0f * gameTime), true);
                }
            }
            else
            {
                // ande na direção selecionada
                Move(Vector(ang, mag * gameTime));
            }

            // angulo de tiro
            float shot_ang = Line::Angle(Point(0, 0), Point(CoopersReturn::gamepad->XboxAnalog(ThumbLX) / 25.0f, -1.0f * (CoopersReturn::gamepad->XboxAnalog(ThumbLY) / 25.0f)));

            bool x = (CoopersReturn::gamepad->XboxAnalog(ThumbLX) > 100 || CoopersReturn::gamepad->XboxAnalog(ThumbLX) < -100);
            bool y = (CoopersReturn::gamepad->XboxAnalog(ThumbLY) > 100 || CoopersReturn::gamepad->XboxAnalog(ThumbLY) < -100);

            if ((x || y) && !atirando )
            {
                atirando = true;
                Move(Vector(shot_ang, 0.0f), false, true);
                CoopersReturn::scene->Add(new Missile(), STATIC);
            }
            if (!x && !y)
            {
                atirando = false;
            }
        }
        else {
            // modifica vetor velocidade do player
            if (window->KeyDown(VK_RIGHT) && window->KeyDown(VK_UP))
                Move(Vector(45.0f, accel));
            else if (window->KeyDown(VK_LEFT) && window->KeyDown(VK_UP))
                Move(Vector(135.0f, accel));
            else if (window->KeyDown(VK_LEFT) && window->KeyDown(VK_DOWN))
                Move(Vector(225.0f, accel));
            else if (window->KeyDown(VK_RIGHT) && window->KeyDown(VK_DOWN))
                Move(Vector(315.0f, accel));
            else if (window->KeyDown(VK_RIGHT))
                Move(Vector(0.0f, accel));
            else if (window->KeyDown(VK_LEFT))
                Move(Vector(180.0f, accel));
            else if (window->KeyDown(VK_UP))
                Move(Vector(90.0f, accel));
            else if (window->KeyDown(VK_DOWN))
                Move(Vector(270.0f, accel));
            else
                // se nenhuma tecla está pressionada comece a frear
                if (speed->Magnitude() > 0.1f)
                    Move(Vector(speed->Angle() + 180.0f, 5.0f * gameTime));
                else
                    // velocidade muita baixa, não use soma vetorial, apenas pare
                    speed->ScaleTo(0.0f);

            // dispara m�ssil
            if (window->KeyPress(VK_SPACE))
            {
                CoopersReturn::scene->Add(new Missile(), STATIC);
            }
        }

        // movimenta objeto pelo seu vetor velocidade
        Translate(speed->XComponent() * 50.0f * gameTime, -speed->YComponent() * 50.0f * gameTime);

        // restringe a �rea do jogo
        if (x < 50)
            MoveTo(50, y);
        if (y < 50)
            MoveTo(x, 50);
        if (x > game->Width() - 50)
            MoveTo(game->Width() - 50, y);
        if (y > game->Height() - 50)
            MoveTo(x, game->Height() - 50);
        break;
    }
    }
}

// ---------------------------------------------------------------------------------

void Player::Draw()
{
    switch (CoopersReturn::state)
    {
    case EXPL:
    {
        animExp->Draw(x, y, Layer::FRONT);
        break;
    }
    case INIT:
    case MESG:
    {
       sprite->Draw(x, y);
       animEng->Draw(x - 53.0f, y, Layer::UPPER);
       break;
    }
    case PLAY:
    case CLIMAX:
    case FINALIZE:
    {
        sprite->Draw(x, y, Layer::MIDDLE, scale, -speed->Angle());
        break;
    }
    }
    

}

// -------------------------------------------------------------------------------

void Player::OnCollision(Object* obj) {
    //COMET, ASTEROID, METEOROID
    if (obj->Type() == COMET || obj->Type() == ASTEROID || obj->Type() == METEOROID) {
        CoopersReturn::state = EXPL;
        CoopersReturn::timer.Reset();
    }
}
