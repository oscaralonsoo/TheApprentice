#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class NoctilumeState {
    IDLE,
    DIVE,
    DEAD
};

class Noctilume : public Enemy
{
public:

    Noctilume();
    ~Noctilume() override;
    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    bool CleanUp() override;
    void Flying(float dt);
    void Dive(float dt);

    float DistanceToPlayer();

private:

    bool playerInRange = false;
    NoctilumeState currentState = NoctilumeState::IDLE;
    PhysBody* physBody = nullptr;
    Animation idleAnim;
    Animation flyingAnim;
    Animation divingDownAnim;
    Animation divingUpAnim;


};
