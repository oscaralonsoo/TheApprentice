#pragma once

#include "Timer.h"

class Player;
class PhysBody;

class AttackMechanic {
public:
    void Init(Player* player);
    void Update(float dt);

    bool IsAttacking() const;

private:
    void StartAttack();
    void UpdateAttackSensor();
    void DestroyAttackSensor();

private:
    Player* player = nullptr;
    PhysBody* attackSensor = nullptr;

    Timer attackTimer;
    float attackDuration = 500.0f; // en milisegundos

    bool isAttacking = false;
};
