#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class BloodrusherState {
    IDLE,       
    ATTACKING,   
    SLIDING,
    DEAD
};

class Bloodrusher : public Enemy
{
public:

	Bloodrusher();
	~Bloodrusher() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;
    void Idle();
    void Attack(float dt);
    void Slide(float dt);
    void OnCollision(PhysBody* physA, PhysBody* physB) override;


private:
    BloodrusherState currentState = BloodrusherState::IDLE;

    Animation idleAnim;
    Animation attackAnim;
    Animation slideAnim;

    float previousDirection;
    Timer timer;
};
