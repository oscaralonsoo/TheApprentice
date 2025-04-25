#pragma once

#include "Broodheart.h"
#include "Enemy.h"
#include "SDL2/SDL.h"

enum class BroodState {
    IDLE,
    CHASING,
    DEAD
};

class Brood : public Enemy
{
public:
    Brood();
    ~Brood() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    // Brood Methods
    void Chase(float dt);
    // Setters
    void SetParent(Broodheart* p) { parent = p; }

private:
    Broodheart* parent = nullptr;

    float timePassed = 0.0f;
    float dirX = 0.0f;
    float dirY = 0.0f;
    float launchSpeed = 0.05f;
    bool launched = false;
    float waveOffset = 0.0f;

    Vector2D direction = { 0, 0 };
    bool playerInRange = false;

    BroodState currentState = BroodState::IDLE;

    PhysBody* physBody = nullptr;
    Animation idleAnim;
};
