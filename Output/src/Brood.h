#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class BroodState {
    IDLE,
    ATTACK,
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

    void Attack(float dt);


public:

private:
    float SpawnCooldown = 0.0f;
    float SpawnTimer = 0.0f;

    bool playerInRange = false;

    BroodState currentState = BroodState::IDLE;
    PhysBody* physBody = nullptr;
    Animation idleAnim;
};
#pragma once
#pragma once
