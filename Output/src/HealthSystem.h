#pragma once
#include "Timer.h"
#include "Physics.h"

class Player;

class HealthSystem {
public:
    void Init(Player* player);
    void Update(float dt);

    void TakeDamage();
    void HandleSpikeDamage();
    void HealFull();

    int GetLives() const;
    float GetVignetteSize() const;

    void AddLife();
    void SetLives(int lives);

    void SetVignetteSize(float size);

    void ApplyKnockback(const Vector2D& sourcePosition);

private:
    void UpdateVignette();
    void CheckDeath();

private:
    Player* player = nullptr;

    int lives = 3;
    float vignetteSize = 300.0f;

    Timer knockbackTimer;
    bool knockbackActive = false;
    float knockbackDuration = 300.0f;
    b2Vec2 knockbackInitialVelocity;
};
