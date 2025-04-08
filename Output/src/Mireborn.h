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
    bool PostUpdate() override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    void SetParameters(pugi::xml_node parameters) override {
        Enemy::SetParameters(parameters);
        tier = parameters.attribute("tier").as_string();
    }

    void Idle(float dt);

    void Walk(float dt);

    void Divide();

public:

private:
    float jumpCooldown = 0.0f;
    float jumpInterval = 750.0f;; //Time Between Jumps
    float jumpForceY = -20.0f;
    float jumpForceX = 5.0f;
    bool hasJumped = false;
    bool isOnGround = false;

    static const int MAX_DIVIDES = 2;

    std::string tier;

    MirebornState currentState = MirebornState::IDLE;
    PhysBody* physBody = nullptr;
    Animation idleAnim;
    Animation walkAnim;

    bool isDivided = false;
};
