#pragma once

#include "Broodheart.h"
#include "Enemy.h"
#include "SDL2/SDL.h"

enum class BroodState {
    IDLE,
    CHASING,
    RETURNING,
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
    bool PostUpdate();
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    // Brood Methods
    void Chase(float dt);
    void UpdateChaseState(float dt);
    // Setters
    void SetParent(Broodheart* parent);

    void ReturnToInitial(float dt);

    Animation flyingAnim;
    Animation deathAnim;
private:
    bool isDead = false;
    Broodheart* broodHeart = nullptr;
    Vector2D playerPos;
    Vector2D initialPosition;   
    float returnSpeed = 0.1f;   
    bool returningToHeart = false;
    float timePassed = 0.0f;
    float waveOffset = 0.0f;
    float distanceToPlayer = 0.0f;
    const float detectRange = 1280.0f;
    float returningSpeed = 0.2f;
    int direction = 1;
    Vector2D nextDirection = { 0, 0 };

    BroodState currentState = BroodState::IDLE;

    int soundDeathId = 0;
    bool deadSoundPlayed = false;
};
