#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"
#include <list>

class Brood;

class Broodheart : public Enemy
{
public:

    Broodheart();
    ~Broodheart() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate() override;
    bool CleanUp() override;

    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    void Spawn();

    void OnBroodDeath(Brood* brood);

private:
    bool playerInRange = false;
    float lastDeltaTime = 0.0f;
    PhysBody* physBody = nullptr;

    Animation idleAnim;
    Animation spawnAnim;
    Animation deathAnim;

    // Brood Spawn
    float spawnCooldown = 0.0f;
    float spawnInterval = 0.0f;
    bool shouldSpawn = false;
    const float spawnRadius = 100.0f;
    std::list<Brood*> broodsAlive;
    bool isBroken = false;
};