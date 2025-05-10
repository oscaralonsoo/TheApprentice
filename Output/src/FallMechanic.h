#pragma once

#include "Timer.h"

class Player;

class FallMechanic {
public:
    void Init(Player* player);
    void Update(float dt);
    void CheckFallStart();
    void OnLanding();
    bool IsFalling() const { return isFalling; }
    bool IsStunned() const { return isStunned; }

private:
    void CheckLanding();

private:
    Player* player = nullptr;

    bool isFalling = false;

    Timer stunTimer;
    float stunDuration = 910.0f; // en milisegundos
    float fallStunThreshold = 34.0f; // velocidad Y para stun

    bool isStunned = false;
};
