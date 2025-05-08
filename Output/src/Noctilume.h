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
    void Attack();
    void Crash();
    void Die();

    void CheckState();

private:
    Vector2D originalPosition;
    NoctilumeState currentState = NoctilumeState::IDLE;

    Animation flyingAnim;
    Animation attackAnim;
    Animation crashAnim;
    Animation dieAnim;

    float idleTime = 0.0f;
    float chaseSpeed = 0.0015f;
    float idleAmplitude = 300.0f;
    float idleFrequency = 0.0015f;
    float followSmoothing = 5.0f;

    float delayedPlayerX = 0.0f;
    float delayedPlayerY = 0.0f;
    float timePassed = 0.0f;
    float lastSinValue = 0.0f;
};
