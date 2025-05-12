#pragma once
#include "Enemy.h"
#include "SDL2/SDL.h"


enum class DreadspireState {
IDLE,
SHOOTING,
RECHARGING,
DEAD
};

class Dreadspire : public Enemy {
public:
    Dreadspire();
    ~Dreadspire() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate();
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    void CheckState();
private:
    void Idle(float dt);
    void Shoot(float dt);
    void Recharge(float dt);
private:
    Vector2D playerPos;
    DreadspireState currentState = DreadspireState::IDLE;

    Animation idleAnim;
    Animation shootingAnim;
    Animation dieAnim;
};