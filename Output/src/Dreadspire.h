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

    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void CheckState();
    void ChangeBodyType();
private:
    void Idle(float dt);
    void Shoot(float dt);
    void Recharge(float dt);
private:
    bool shouldBecomeStatic = false;
    bool shouldBecomeDynamic = false;

    Vector2D playerPos;
    DreadspireState currentState = DreadspireState::IDLE;

    Animation idleAnim;
    Animation shootingAnim;
    Animation dieAnim;
};