#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class NoctilumeState {
    IDLE,
    CHASING,
    ATTACK,
    CRASH,
    DEAD
};

class Noctilume : public Enemy
{
public:
    Noctilume();
    ~Noctilume() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate();
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    void Idle(float dt);
    void Chasing(float dt);
    void Attack(float dt);
    void Crash(float dt);
    void Die();

    void CheckState();

private:
    Vector2D originalPosition;
    Vector2D playerPos;
    NoctilumeState currentState = NoctilumeState::IDLE;

    Vector2D smoothedPosition; 
    float smoothingSpeed = 0.001f; 

    Animation flyingAnim;
    Animation attackAnim;
    Animation crashAnim;
    Animation dieAnim;

    //Idle 
    const float horizontalRange = 500.0f;
    const float waveAmplitude = 5.0f;
    const float waveFrequency = 0.00035f;
    float waveOffset = 0.0f; 

    //Chase 
    const float hoverHeight = 350;
    const float oscillationAmplitude = 250.0f;
    const float oscillationSpeed = 0.002f;
    float delayedPlayerX = 0.0f;
    float delayedPlayerY = 0.0f;
    float timePassed = 0.0f;
    float lastSinValue = 0.0f;


    //Attack 
    bool isDiving = true;
    float diveProgress = 0.0f;
    float attackSpeed = 0.50f; // Velocidad vertical
    float returnSpeed = 0.30f; // Velocidad al subir después del impacto con el jugador
    Vector2D diveStartPos;
    Vector2D attackTarget;
    float oscillationCount = 0.0f;
    int oscillationsBeforeAttack = 3;
    bool passedZero = false;

    //Crash
    float crashTimer = 0.0f;

};
