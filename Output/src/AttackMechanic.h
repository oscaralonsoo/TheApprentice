#pragma once

#include "Timer.h"
#include <SDL2/SDL_gamecontroller.h>

class Player;
class PhysBody;

class AttackMechanic {
public:
    void Init(Player* player);
    void Update(float dt);

    bool IsAttacking() const;

    void SetController(SDL_GameController* controller);

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

    SDL_GameController* controller = nullptr;
    bool attackHeldPreviously = false;
};
