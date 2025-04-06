#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"


class Broodheart : public Enemy
{
public:

    Broodheart();

    ~Broodheart() override;

    bool Awake() override;

    bool Start() override;

    bool Update(float dt) override;

    bool PostUpdate();

    bool CleanUp() override;

    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    void Spawn();

    int broodCount = 0;
private:

    bool playerInRange = false;

    PhysBody* physBody = nullptr;

    Animation idleAnim;

    //Summon

    float spawnCooldown = 0.0f;
    float spawnInterval = 5000.0f;
    bool shouldSpawn = false;
    const float spawnRadius = 150.0f;

};
#pragma once
