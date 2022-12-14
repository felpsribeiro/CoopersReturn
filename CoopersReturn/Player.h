
#ifndef _COOPER_PLAYER_H_
#define _COOPER_PLAYER_H_

// ---------------------------------------------------------------------------------

#include "Object.h"                        // objetos do jogo
#include "Sprite.h"                        // desenho de sprites
#include "Vector.h"                        // representa��o de vetores
#include "Particles.h"                     // sistema de part�culas
#include "Animation.h"                     // sistema de animações

// ---------------------------------------------------------------------------------

class Player : public Object
{
private:
    Sprite * sprite;                    // sprite do objeto
    TileSet * engine;                   // motor
    Animation * animEng;                   // animação do motor
    float scale = 1.0f;
    bool atirando = false;

    TileSet* explosion;                   // explosçao
    Animation* animExp;                   // animação da explosão

public:
    Vector * speed;                     // velocidade e dire��o

    Player();                           // construtor
    ~Player();                          // destrutor
    
    void Init();                        // set as config iniciais
    void Move(Vector && v, bool freio, bool shot);             // movimenta jogador
    void Update();                      // atualiza��o
    void Draw();                        // desenho
    void OnCollision(Object* obj);
}; 
// ---------------------------------------------------------------------------------

#endif