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
    void Attack();
    void Impaled();
    void Roar();
    void ChangeImpaledAnim();
    void UpdateDraw();
    void UpdateColliderSizeToCurrentAnimation();
    void TriggerRoar();
    float GetDifficultyMultiplier() const;
    float GetAttackSpeed() const;
public:

    NullwardenState currentState = NullwardenState::IDLE;
    bool crystalBroken = false;
    Animation* currentAnimation;
    Animation idleAnim;
    Animation attackAnim;
    Animation chargeAnim;
    Animation roarAnim;
    Animation hitAnim;
    Animation deathAnim;

    Animation crystalAppearAnim1;
    Animation crystalAppearAnim2;
    Animation crystalAppearAnim3;

    Animation impaledAnim;
    Animation impaledAnim1;
    Animation impaledAnim2;
    Animation impaledAnim3;

    bool startedImpaledAnim = false;
    int direction = 1;
private:

    int drawY = 0;
    int drawX = 0;
    Animation* previousAnimation = nullptr;
    NullwardenState previousState = NullwardenState::IDLE;
    bool changedDirection = false;
    NullwardenCrystal* crystal= nullptr;
    bool attackAnimDone = false;
    const float spearIntervalMs = 2000.0f;
    Timer spearIntervalTimer;
    const float spearAttackMs = 15000.0f;
    Timer spearAttackTimer;
    const float impaledMs = 10000.0f;
    Timer beforeChargeTimer;
    const float beforeChargeMs = 1100.0f;
    Timer impaledTimer;
    Timer verticalSpearTimer;
    const float verticalSpearIntervalMs = 300.0f;
    int spawnedVerticalSpears = 0;
    const int maxVerticalSpears = 8;
    const float verticalSpearGap = 200.0f;

};
