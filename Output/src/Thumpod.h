#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class ThumpodState {
    IDLE,
    DEAD
};

class Thumpod : public Enemy
{
public:

	Thumpod();
	~Thumpod() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;
    void Idle();
    void OnCollision(PhysBody* physA, PhysBody* physB) override;


private:
    ThumpodState currentState = ThumpodState::IDLE;

    Animation idleAnim;

    float previousDirection;
    Timer timer;
};
