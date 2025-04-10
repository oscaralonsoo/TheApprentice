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
    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    bool CleanUp() override;

    void IdleFlying(float dt);

    void Flying(float dt);

    void Dive(float dt);

    float DistanceToPlayer();

private:

    bool playerInRange = false;
    NoctilumeState currentState = NoctilumeState::IDLE;


    PhysBody* physBody = nullptr;
    Animation idleAnim;
    Animation flyingAnim;
    Animation divingDownAnim;
    Animation divingUpAnim;

    float idleTime = 0.0f;
    float sineOffset = 0.0f;
    float waveOffset = 0.0f;
    float timePassed = 0.0f;
    float diveProgress = 0.0f;

    Vector2D direction = { 0.0f, 0.0f };

    float attackCooldown = 3000.0f;
    float attackTimer = 0.0f;
    float diveTime = 0.0f;
    float proximityDuration = 2000.0f;
    float proximityTimer = 0.0f;
    Vector2D diveStartPos;
    Vector2D diveTargetPos;
    bool divingDown = true;
    Vector2D lastDivePosition;              // Última posición al salir del dive
    bool justFinishedDive = false;          // Marca si acaba de salir del dive
    float flyingTransitionTimer = 0.0f;     // Temporizador para blend suave
    const float flyingTransitionDuration = 0.5f; // Duración del blend
    float diveElapsedTime = 0.0f;

};
