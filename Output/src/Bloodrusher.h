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
    bool PostUpdate() override;
    bool CleanUp() override;
    void Idle();
    void Attack(float dt);
    void Slide(float dt);
    void Dead();
    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    BloodrusherState GetCurrentState() const { return currentState; }
    void SetState(BloodrusherState state) { currentState = state; }
    bool IsGroundAhead();


private:
    BloodrusherState currentState = BloodrusherState::IDLE;

    Animation idleAnim;
    Animation attackAnim;
    Animation slideAnim;
    Animation deadAnim;

    float previousDirection;
    Timer timer;
    Timer turnTimer;
    float turnDelay = 0.5f; 
    bool turning = false;
    float exponentialFactor = 1.007f;
    float velocityBase = 0.15f;
    float maxSpeed = 15.0f;

    int soundMoveId = 0;
    int soundSlideId = 0;
    int soundDeadId = 0;

    bool moveSoundPlayed = false;
    bool slideSoundPlayed = false;
    bool deadSoundPlayed = false;

    bool isDead = false;
};
