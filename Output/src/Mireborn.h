#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class MirebornState {
    IDLE,       
    ATTACKING,   
    SLIDING,
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

public:

private:
    MirebornState currentState = MirebornState::IDLE;

    Animation idleAnim;
    Animation attackAnim;
    Animation slideAnim;
};
