#pragma once

#include "Timer.h"
#include "Vector2D.h"
#include <SDL2/SDL_gamecontroller.h>

class Player;

class DashMechanic {
public:
    void Init(Player* player);
    void Update(float dt);

    void Enable(bool enable);
    bool IsDashing() const;

    void CancelDash();            
    void OnWallCollision();  

    bool IsDashUnlocked() const { return dashUnlocked; }

    void SetController(SDL_GameController* controller); // en public

private:
    void StartDash();
    void ApplyDashMovement();

private:
    Player* player = nullptr;

    bool dashUnlocked = false;
    bool isDashing = false;
    bool canDash = true;

    int dashDirection = 1;
    float dashSpeed = 15.0f;
    float maxDashDistance = 165.0f;

    Vector2D dashStartPosition;

    Timer dashCooldownTimer;
    float dashCooldownTime = 1.0f;

    SDL_GameController* controller = nullptr; // en private
    bool rtHeldPreviously = false;
    Vector2D lastPosition;
    int stuckFrameCounter = 0;
    const int stuckFrameLimit = 5;  // Número de frames sin moverse antes de cancelar el dash

    bool transitionToDash = false;

    int soundDashId = 0;
};
