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
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    void Idle(float dt);

    void Chase(float dt);


    void ReturnToPlayer(float dt);

private:
    float speed = 4.0f;
    float circleAngle = 0.0f;
    float returnRadius = 60.0f;
    float angularSpeed = 2.0f;
    Vector2D returnCenter;
    float returnStartAngle = 0.0f;
    Vector2D lastDirection;

    bool playerInRange = false;

    BroodState currentState = BroodState::IDLE;
    PhysBody* physBody = nullptr;
    Animation idleAnim;
};
#pragma once
