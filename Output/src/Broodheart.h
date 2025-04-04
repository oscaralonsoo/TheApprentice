#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class BroodheartState {
    IDLE,
    SPAWN,
    DEAD
};

class Broodheart : public Enemy
{
public:

    Mireborn();
    ~Mireborn() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    void Idle(float dt);

    void Walk(float dt);


public:

private:
    float SpawnCooldown = 0.0f;
    float SpawnTimer = 0.0f;

    bool playerInRange = false;
    int size = 128;

    BroodheartState currentState = BroodheartState::IDLE;
    PhysBody* physBody = nullptr;
    Animation idleAnim;
    Animation walkAnim;
};
#pragma once
