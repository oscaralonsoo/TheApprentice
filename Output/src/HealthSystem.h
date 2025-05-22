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
    int GetMaxLives() const;
    float GetVignetteSize() const;

    void AddLife();
    void SetLives(int lives);

    void AddMaxLife();
    void SetMaxLives(int lives);

    void SetVignetteSize(float size);

    void ApplyKnockback(const Vector2D& sourcePosition);

    bool IsInHitAnim() const { return isInHitAnim; }

    bool IsDying() const { return isDying; }

private:
    void UpdateVignette();
    void CheckDeath();

private:
    Player* player = nullptr;

    int lives = 3;
    int maxlives = 3;
    float vignetteSize = 300.0f;

    Timer knockbackTimer;
    bool knockbackActive = false;
    float knockbackDuration = 300.0f;
    b2Vec2 knockbackInitialVelocity;

    Timer hitTimer;
    bool isInHitAnim = false;
    float hitAnimDuration = 350.0f;

    bool isDying = false;
    Timer deathTimer;
    float deathAnimDuration = 1000.0f; // Ajusta según tu animación
};
