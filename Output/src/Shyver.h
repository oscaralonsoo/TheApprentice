#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class ShyverState {
    IDLE,
    APPEAR,
    WAIT,
    PRE_ATTACK,
    ATTACK,
    STUNNED,
    DISAPPEAR,
    INVISIBLE,
    DEATH
};

class Shyver : public Enemy
{
public:

	Shyver();
	~Shyver() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate() override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB);

    void UpdateColliderSizeToCurrentAnimation();

    void Wait();
    void Attack();
    void Stun();
    void Disappear();
    void Death();
    void Invisible();

private:
    ShyverState currentState = ShyverState::IDLE;

    Animation* previousAnimation = nullptr;
    Animation idleAnim;
    Animation invisibleAnim;
    Animation stunAnim;
    Animation waitAnim;
    Animation disappearAnim;
    Animation appearAnim;
    Animation attackAnim;
    Animation preattackAnim;
    Animation deathAnim;
    Animation waveAnim;

    Animation* secondAnimation= nullptr;

    const float waitDuration = 2000.0f;
    Timer waitTimer;
    const float invisibleDuration = 2000.0f;
    Timer invisibleTimer;
    const float stunDuration = 2000.0f;
    Timer stunTimer;

    Vector2D waveAnimStartPos;

    float attackStartX = 0.0f;
    bool attackInProgress = false;

    int soundWalkId = 0;
    int soundDeadId = 0;
    int soundDashId = 0;
    int soundAppearId = 0;
    bool printWave = false;
    bool walkSoundPlayed = false;
    bool deadSoundPlayed = false;
    bool dashSoundPlayed = false;
    bool appearSoundPlayed = false;
};
