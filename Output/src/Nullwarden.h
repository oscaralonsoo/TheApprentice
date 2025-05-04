#pragma once

#include "Enemy.h"
#include "NullwardenSpear.h"
#include "SDL2/SDL.h"
#include "Timer.h"

class Spear;

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

    void SpawnSpears(bool isHorizontal);
    void PushPlayerBack();

private:
    NullwardenState currentState = NullwardenState::IDLE;
    
    Animation idleAnim;
    Animation attackAnim;
    Animation chargeAnim;
    Animation impaledAnim;
    Animation roarAnim;
    Animation deathAnim;

    const float spearIntervalMs = 1600.0f;
    Timer spearIntervalTimer;
    const float spearAttackMs = 1000.0f;
    Timer spearAttackTimer;
    const float impaledMs = 2000.0f;
    Timer impaledTimer;

    int lives = 3;
};

