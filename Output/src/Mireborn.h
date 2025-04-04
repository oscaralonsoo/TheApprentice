#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class MirebornState {
    IDLE,         
    WALKING,
    DEAD
};

class Mireborn : public Enemy
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

    void Divide();

public:

private:
    float jumpCooldown = 0.0f;
    float jumpInterval = 750.0f;; //Time Between Jumps
    float jumpForceY = -30.0f;
    float jumpForceX = 20.0f;
    bool hasJumped = false;
    bool isOnGround = false;

    bool playerInRange = false;
    int size = 128;

    MirebornState currentState = MirebornState::IDLE;
    PhysBody* physBody = nullptr;
    Animation idleAnim;
    Animation walkAnim;
};
