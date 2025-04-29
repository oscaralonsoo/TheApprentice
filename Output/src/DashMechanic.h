#pragma once

#include "Timer.h"
#include "Vector2D.h"

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
    float maxDashDistance = 100.0f;

    Vector2D dashStartPosition;

    Timer dashCooldownTimer;
    float dashCooldownTime = 1.0f;
};
