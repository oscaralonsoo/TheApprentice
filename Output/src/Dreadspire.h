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

    bool shouldBecomeStatic = false;
    bool shouldBecomeDynamic = false;

    float angles[3] = { -M_PI / 10, 0.0f, M_PI / 10 };
    float spawnOffset = 90.0f;
    float bulletShootTimer = 0.0f;
    int bulletsShot = 0;
    float baseAngle = 0.0f;
    float rechargeTimer = 0.0f;
    float rechargeCooldown = 2000.0f; 
    Vector2D playerPos;
    DreadspireState currentState = DreadspireState::IDLE;

    Animation idleAnim;
    Animation shootingAnim;
    Animation dieAnim;

    int soundAttackId = 0;
    int soundDeadId = 0;
    int soundRechargeId = 0;

    bool attackSoundPlayed = false;
    bool deadSoundPlayed = false;
    bool rechargeSoundPlayed = false;
};