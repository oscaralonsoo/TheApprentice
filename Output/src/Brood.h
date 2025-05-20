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
    bool PostUpdate(float dt);
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    // Brood Methods
    void Chase(float dt);
    void UpdateChaseState();
    // Setters
    void SetParent(Broodheart* p) { broodHeart = p; }

    Animation flyingAnim;
    Animation deathAnim;
private:
    bool isDead = false;
    Broodheart* broodHeart = nullptr;

    float detectionRange = 100000.0f;
    float timePassed = 0.0f;
    float waveOffset = 0.0f;
    float distanceToPlayer = 0.0f;

    Vector2D direction = { 0, 0 };

    BroodState currentState = BroodState::IDLE;

};
