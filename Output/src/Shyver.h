#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class ShyverState {
    IDLE,
    INVISIBLE,
    APPEAR,
    STUNNED,
    DISAPPEAR,
    ATTACK,
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

    void Appear();
    void Attack();
    void Stun();
    void Disappear();
    void Death();
    void Invisible();

private:
    ShyverState currentState = ShyverState::IDLE;

    Animation idleAnim;
    Animation invisibleAnim;
    Animation stunAnim;
    Animation disappearAnim;
    Animation appearAnim;
    Animation attackAnim;
    Animation deathAnim;

    const float appearDuration = 2000.0f;
    Timer appearTimer;

    float attackStartX = 0.0f;
    bool attackInProgress = false;


};
