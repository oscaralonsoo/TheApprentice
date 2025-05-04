#pragma once

#include "Timer.h"

class Player;

class FallMechanic {
public:
    void Init(Player* player);
    void Update(float dt);
    void OnLanding();
    bool IsFalling() const { return isFalling; }

private:
    void CheckFallStart();
    void CheckLanding();
    void ApplyFallStunIfNeeded();

private:
    Player* player = nullptr;

    bool isFalling = false;
    bool willStun = false;

    Timer stunTimer;
    float stunDuration = 910.0f; // en milisegundos
    float fallStunThreshold = 1.0f; // velocidad Y para stun

    bool isStunned = false;
};
