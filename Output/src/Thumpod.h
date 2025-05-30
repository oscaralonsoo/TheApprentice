#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"
#include <vector>
#include <box2d/box2d.h> 
enum class ThumpodState {
    IDLE,
    ATTACK,
    DEAD
};

class Thumpod : public Enemy {
public:
    Thumpod();
    ~Thumpod() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate() override;
    bool CleanUp() override;

    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    void UpdateDirectionFacingPlayer();


private:
    void Idle();
    void Attack(float dt);
    void TryStartJump(float mass);
    void TryChangeToFallingState(float mass);
    void TryFinishAttackDown(float dt);
    void ResetAttackState();
    void Die();

    ThumpodState currentState = ThumpodState::IDLE;

    float previousDirection = 0.0f;
    std::vector<b2Vec2> shape;

    Animation idleAnim;
    Animation attackUpAnim;
    Animation attackDownAnim;
    Animation deadAnim;

    float jumpCooldown = 0.0f;
    float attackDownTimer = 0.0f;

    const float jumpInterval = 750.0f;
    const float minAttackDownTime = 500.0f;
    const float jumpForceY = -12.0f;

    bool hasJumped = false;
    bool isOnGround = false;
    bool isJumpingUp = false;
    bool isFallingTowardsPlayer = false;
    bool canPlayAttackDownAnim = false;
    bool hasLanded = false;
    bool isPlayingAttackDown = false;
};
