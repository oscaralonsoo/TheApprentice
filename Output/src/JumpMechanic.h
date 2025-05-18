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

    void HandleJumpInput(float dt);
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
    bool jumpInterrupted = false;

    float minJumpForce = 100.0f;             // Fuerza inicial para garantizar altura mínima
    float progressiveJumpForce = 220.0f;     // Fuerza para el salto sostenido
    float jumpHoldForceFactor = 1.7f;       // Factor inicial de fuerza sostenida
    float jumpDecayRate = 6.5f;             // Qué tan rápido decae la fuerza sostenida
    float fallAccelerationFactor = 0.0f;   // Fuerza que empuja hacia abajo al soltar el salto

    Timer jumpHoldTimer;
    float jumpHoldDuration = 370.0f;

    bool controllerHeldPreviously = false;
    bool keyboardHeldPreviously = false;

    Timer jumpCooldownTimer;
    float jumpCooldownTime = 100.0f;
    bool jumpCooldownActive = false;

    SDL_GameController* controller = nullptr;
};
