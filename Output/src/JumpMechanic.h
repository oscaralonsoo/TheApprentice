#pragma once

#include "Timer.h"
#include <SDL2/SDL_gamecontroller.h>

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

    bool IsJumpUnlocked() const { return jumpUnlocked; }
    bool IsDoubleJumpUnlocked() const { return doubleJumpUnlocked; }
    void SetJumpCount(int count) { jumpCount = count; }

    void SetController(SDL_GameController* controller);

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
    bool controllerHeldPreviously = false;

    Timer jumpCooldownTimer;
    float jumpCooldownTime = 100.0f;
    bool jumpCooldownActive = false;

    SDL_GameController* controller = nullptr;

    bool keyboardHeldPreviously = false;

};
