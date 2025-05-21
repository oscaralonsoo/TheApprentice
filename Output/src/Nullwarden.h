#pragma once

#include "Enemy.h"
#include "NullwardenSpear.h"
#include "NullwardenCrystal.h"
#include "SDL2/SDL.h"
#include "Timer.h"

class Spear;

class NullwardenCrystal;

enum class NullwardenState {
    IDLE,
    ATTACK,
    CHARGE,
    IMPALED,
    ROAR,
    DEATH
};

class Nullwarden : public Enemy
{
public:
    Nullwarden();
    ~Nullwarden() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate() override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB);

    void SpawnHorizontalSpears();
    void SpawnVerticalSpears();
    b2Vec2 GetCrystalOffset() const;
    float GetCrystalRotation() const;
    void Attack();
    void Impaled();
    void Roar();
public:

    NullwardenState currentState = NullwardenState::IDLE;
    bool crystalBroken = false;
    Animation* currentAnimation;
    Animation idleAnim;
    Animation attackAnim;
    Animation chargeAnim;
    Animation impaledAnim;
    Animation roarAnim;
    Animation deathAnim;

private:
    float drawOffset = 0.0f;
    float idleCrystalOffsetTimer = 0.0f;
    NullwardenCrystal* crystal= nullptr;

    const float spearIntervalMs = 2000.0f;
    Timer spearIntervalTimer;
    const float spearAttackMs = 100.0f;
    Timer spearAttackTimer;
    const float impaledMs = 20000.0f;
    Timer impaledTimer;
    const float roarMs = 2000.0f;
    Timer roarTimer;
    Timer verticalSpearTimer;
    const float verticalSpearIntervalMs = 300.0f;
    int spawnedVerticalSpears = 0;
    const int maxVerticalSpears = 15;
    const float verticalSpearGap = 80.0f;

};
