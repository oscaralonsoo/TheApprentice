#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class MirebornState {
    IDLE,         
    WALKING,
    DIVIDING,
    DEATH
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
    float jumpForceY = -4.0f;
    float jumpForceX = 4.0f;
    bool isOnGround = false;
    const float jumpCooldown = 1600.0f;
    Timer jumpCooldownTimer;

    static const int MAX_DIVIDES = 2;

    std::string tier;

    MirebornState currentState = MirebornState::WALKING;
    PhysBody* physBody = nullptr;
    Animation idleAnim;
    Animation walkAnim;
    Animation divideAnim;
    Animation deathAnim;

    bool isDivided = false;
    
    int soundDivideId = 0;
    int soundDeathId = 0;
    int soundWalkId = 0;

    bool divideSoundPlayed = false;
    bool deathSoundPlayed = false;
    bool walkSoundPlayed = false;

};
