#pragma once

#include "Timer.h"

class Player;

class JumpMechanic {
public:
    void Init(Player* player);
    void Update(float dt);

    void Enable(bool enable);
    void EnableDoubleJump(bool enable);

    void OnLanding();
    void OnLeaveGround();

    void HandleJumpInput();
    bool IsJumping() const { return isJumping; }

private:
    Player* player = nullptr;

    bool jumpUnlocked = false;
    bool doubleJumpUnlocked = false;

    int jumpCount = 0;
    const int maxJumpCount = 2;

    bool isJumping = false;
    bool isHoldingJump = false;

    float jumpForce = 13.0f;
    float fallAccelerationFactor = 0.6f;

    float jumpStartY = 0.0f;
    float maxJumpHeight = 240.0f;
    float minHoldJumpHeight = 20.0f;
    float jumpHoldForceFactor = 0.85f;
    float jumpDecayRate = 4.0f;

    Timer jumpCooldownTimer;
    float jumpCooldownTime = 100.0f;
    bool jumpCooldownActive = false;
};
