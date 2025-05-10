#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class NoctilumeState {
    IDLE,
    CHASING,
    PRE_ATTACK,
    ATTACK,
    CRASH,
    DEAD
};

class Noctilume : public Enemy {
public:
    Noctilume();
    ~Noctilume() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate();
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

private:
    void Idle(float dt);
    void Chasing(float dt);
    void PreAttack(float dt);
    void Attack(float dt);
    void Crash(float dt);
    void Die();
    void CheckState();

    Vector2D GetBodyPosition() const;

private:
    NoctilumeState currentState = NoctilumeState::IDLE;

    Vector2D originalPosition;
    Vector2D smoothedPosition;
    Vector2D playerPos;

    float smoothingSpeed = 0.001f;

    Animation flyingAnim;
    Animation attackAnim;
    Animation crashAnim;
    Animation dieAnim;

    // Idle movement
    const float horizontalRange = 500.0f;
    const float waveAmplitude = 5.0f;
    const float waveFrequency = 0.00035f;
    float waveOffset = 0.0f;

    // Chasing
    const float hoverHeight = 350.0f;
    const float oscillationAmplitude = 350.0f;
    const float oscillationSpeed = 0.002f;
    float delayedPlayerX = 0.0f;
    float delayedPlayerY = 0.0f;
    float timePassed = 0.0f;
    float lastSinValue = 0.0f;
    float oscillationCount = 0.0f;
    int oscillationsBeforeAttack = 3;
    bool passedZero = false;

    // Pre-attack
    Vector2D preAttackStartPos;
    Vector2D preAttackEndPos;
    float anticipationTimer = 0.0f;
    const float anticipationDuration = 500.0f;

    // Attack
    bool isDiving = true;
    float diveProgress = 0.0f;
    float attackSpeed = 0.70f;
    float returnSpeed = 0.50f;
    Vector2D diveStartPos;
    Vector2D attackTarget;
    Vector2D diveDirection;

    // Crash
    float crashTimer = 0.0f;
};
