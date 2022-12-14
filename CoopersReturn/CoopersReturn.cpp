
#include "Resources.h"
#include "CoopersReturn.h"
#include "Engine.h"
#include "Menu.h"

// ------------------------------------------------------------------------------

Player * CoopersReturn::player  = nullptr;
Audio  * CoopersReturn::audio   = nullptr;
Scene  * CoopersReturn::scene   = nullptr;
uint     CoopersReturn::state;
Timer    CoopersReturn::timer;
Controller * CoopersReturn::gamepad = new Controller();
bool CoopersReturn::ctrl = false;

// ------------------------------------------------------------------------------

void CoopersReturn::Init() 
{
    // cria sistema de �udio
    audio = new Audio();
    audio->Add(INTRO, "Resources/Dust.wav");
    audio->Add(END, "Resources/FirstStep.wav");
    audio->Volume(INTRO, 1.0f);
    audio->Volume(END, 1.0f);
    audio->Play(INTRO, true);

    font1 = new Font("Resources/SimSun16.png");
    font1->Spacing("Resources/SimSun16.dat");
    font2 = new Font("Resources/Roman16.png");
    font2->Spacing("Resources/Roman16.dat");

    // carrega/incializa objetos
    backg1  = new Background("Resources/Space.jpg", window->CenterX(), window->CenterY());
    backg2  = new Background("Resources/Black.jpg", window->CenterX(), window->CenterY());
    player  = new Player();
    scene   = new Scene();

    // adiciona objetos na cena
    scene->Add(new Menu(), STATIC);
    scene->Add(player, MOVING);

    obstacle = new Obstacle();

    state = INIT;

    // ----------------------
    // inicializa a viewport
    // ----------------------

    // calcula posi��o para manter viewport centralizada
    float difx = (game->Width() - window->Width()) / 2.0f;
    float dify = (game->Height() - window->Height()) / 2.0f;

    // inicializa viewport para o centro do mundo
    viewport.left = 0.0f + difx;
    viewport.right = viewport.left + window->Width();
    viewport.top = 0.0f + dify;
    viewport.bottom = viewport.top + window->Height();

    ctrl = gamepad->XboxInitialize();
}

// ------------------------------------------------------------------------------

void CoopersReturn::Update()
{
    // sai com o pressionamento da tecla ESC
    if (window->KeyDown(VK_ESCAPE))
        window->Close();

    // atualiza cena e calcula colis�es
    scene->Update();
    scene->CollisionDetection();

    // ativa ou desativa a bounding box
    if (window->KeyPress('B'))
        viewBBox = !viewBBox;

    // ativa ou desativa o heads up display
    gamepad->XboxUpdateState();
    if ((window->KeyPress(VK_RETURN) || gamepad->XboxButton(ButtonStart)) && state == INIT)
    {
        state = MESG;
        timer.Reset();
        auxTimer.Start();
    }

    // --------------------
    // atualiza a viewport
    // --------------------

    viewport.left   = player->X() - window->CenterX();
    viewport.right  = player->X() + window->CenterX();
    viewport.top    = player->Y() - window->CenterY();
    viewport.bottom = player->Y() + window->CenterY();
            
    if (viewport.left < 0)
    {
        viewport.left  = 0;
        viewport.right = window->Width();
    }
    else if (viewport.right > game->Width())
    {  
        viewport.left  = game->Width() - window->Width();
        viewport.right = game->Width();
    }
                  
    if (viewport.top < 0)
    {
        viewport.top  = 0;
        viewport.bottom = window->Height();
    }
    else if (viewport.bottom > game->Height())
    {
        viewport.top = game->Height() - window->Height();
        viewport.bottom = game->Height();
    }

    // ------------------------------------------------
    // gerencia elementos de acordo com o tempo de jogo
    // ------------------------------------------------
    switch (state)
    {
    case MESG:
    {
        if (timer.Elapsed(5.0f)) // primeira etapa do jogo -> 0:00 à 0:59
        {
            state = PLAY;
        }
        break;
    };
    case PLAY:
    case CLIMAX:
    {
        if (!timer.Elapsed(60.0f)) // primeira etapa do jogo -> 0:00 à 0:59
        {
            if (auxTimer.Elapsed(4.0f))
            {
                obstacle->Generate(ASTEROID, 50.0f);
                auxTimer.Reset();
            }
        }
        else if (!timer.Elapsed(120.0f)) // segunda etapa do jogo -> 1:00 à 1:59
        {
            if (auxTimer.Elapsed(3.0f))
            {
                obstacle->Generate(ASTEROID, 50.0f);
                obstacle->Generate(METEOROID, 30.0f);
                auxTimer.Reset();
            }
        }
        else if (!timer.Elapsed(180.0f)) // terceira etapa do jogo -> 2:00 à 2:59
        {
            if (auxTimer.Elapsed(3.0f))
            {
                //obstacle->Generete(ASTEROID, 30.0f);
                obstacle->Generate(ASTEROID, 50.0f);
                obstacle->Generate(METEOROID, 30.0f);
                obstacle->Generate(COMET, 200.0f);
                auxTimer.Reset();
            }

            if (state != CLIMAX && timer.Elapsed(128.0f))
            {
                state = CLIMAX;

                audio->Stop(INTRO);
                audio->Play(END, false);
            }
        }
        else if (state != FINALIZE)
        {
            state = FINALIZE;
            BlackHole* hole = new BlackHole();
            scene->Add(hole, STATIC);
        }
        break;
    }
    case EXPL:
    {
        if (timer.Elapsed(2.0f))
        {
            state = LOST;
        }
        break;
    }
    case LOST:
    {
        if (timer.Elapsed(5.0f))
        {
            state = RESTART;
        }
        break;
    }
    case RESTART:
    {
        state = INIT;
            
        scene->DeleteAll();

        player = new Player();

        // adiciona objetos na cena
        scene->Add(new Menu(), STATIC);
        scene->Add(player, MOVING);

        audio->Stop(END);
        audio->Stop(INTRO);
        audio->Play(INTRO);
        break;
    }
    }
} 

// ------------------------------------------------------------------------------

void CoopersReturn::Restart()
{
    
}

// ------------------------------------------------------------------------------

void CoopersReturn::Draw()
{
    if (state == LOST)
    {
        font1->Draw(window->CenterX() - 150.0f, window->CenterY(), "Lembre-se da Lei de Murphy:", textColor);
        font1->Draw(window->CenterX() - 150.0f, window->CenterY() + 20.0f, "Qualquer coisa que possa ocorrer mal,", textColor);
        font1->Draw(window->CenterX() - 150.0f, window->CenterY() + 40.0f, "ocorrera mal, no pior momento possivel.", textColor);
    }
    else
    {
        if (state == PLAY || state == CLIMAX)
        {
            text.str("");
            text << "Tempo para colisão com Gargantua: " << int(190.0f - timer.Elapsed()) << " s";
            font1->Draw(50.0f, 20.0f, text.str(), textColor, Layer::UPPER);
        }
        
        backg1->Draw(viewport);

        // desenha a cena
        scene->Draw();

        // desenha bounding box
        if (viewBBox)
            scene->DrawBBox();
    }
}

// ------------------------------------------------------------------------------

void CoopersReturn::Finalize()
{
    delete audio;
    delete scene;
    delete backg1;
    delete backg2;
    delete gamepad;
}

// ------------------------------------------------------------------------------
//                                  WinMain                                      
// ------------------------------------------------------------------------------

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, 
                     _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    // cria motor do jogo
    Engine * engine = new Engine();

    // configura motor
    //engine->window->Mode(WINDOWED);
    //engine->window->Size(1152, 648);
    engine->window->Mode(BORDERLESS);
    engine->window->Color(0, 0, 0);
    engine->window->Title("CoopersReturn");
    engine->window->Icon(IDI_ICON);
    engine->window->Cursor(IDC_CURSOR);
    engine->window->HideCursor(true);
    //engine->graphics->VSync(true);

    // cria o jogo
    Game * game = new CoopersReturn();

    // configura o jogo
    game->Size(2100, 1181);
    
    // inicia execu��o
    int status = engine->Start(game);

    // destr�i motor e encerra jogo
    delete engine;
    return status;
}

// ----------------------------------------------------------------------------