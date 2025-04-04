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

    Broodheart();
    ~Broodheart() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    void Idle(float dt);

    void Spawn(float dt);


public:

private:
    float SpawnCooldown = 0.0f;
    float SpawnTimer = 0.0f;

    bool playerInRange = false;

    BroodheartState currentState = BroodheartState::IDLE;
    PhysBody* physBody = nullptr;
    Animation idleAnim;
};
#pragma once
