#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class NoctilumeState {
    IDLE,
    FLYING,
    DIVE,
    DIVE_COOLDOWN,
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

    void IdleFlying(float dt);
    void Flying(float dt);
    void Dive(float dt);

    float DistanceToPlayer();
    void HandleStateTransition();
    void TryInitiateDiveAttack(float dt);

private:
    bool playerInRange = false;
    NoctilumeState currentState = NoctilumeState::IDLE;

    Animation idleAnim;
    Animation flyingAnim;
    Animation divingDownAnim;
    Animation divingUpAnim;

    // Movement & Flight
    float timePassed = 0.0f;
    float sineOffset = 0.0f;
    float waveOffset = 0.0f;
    float flyingTransitionTimer = 0.0f;
    const float flyingTransitionDuration = 0.5f;

    // Position & Oscilation
    float lastSinValue = 0.0f;
    Vector2D direction = { 0.0f, 0.0f };
    float idleTime = 0.0f;

    float attackCooldown = 3000.0f;
    float attackTimer = 0.0f;
    float proximityDuration = 2000.0f;
    float proximityTimer = 0.0f;

    // Dive
    Vector2D diveControlPos;
    Vector2D diveStartPos;
    Vector2D diveTargetPos;
    Vector2D diveReferencePlayerPos;
    Vector2D lastDivePosition;
    float diveElapsedTime = 0.0f;
    float diveTime = 0.0f;
    float diveProgress = 0.0f;
    bool divingDown = true;
    bool justFinishedDive = false;

    float delayedPlayerX = 0.0f;
    float delayedPlayerY = 0.0f;
};
