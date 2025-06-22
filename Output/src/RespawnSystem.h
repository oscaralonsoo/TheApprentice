#pragma once

#include "Vector2D.h"

class Player;
class PhysBody;

class RespawnSystem {
public:
    void Init(Player* player);
    void Update(float dt);

    void UpdateLastSafePosition(PhysBody* platformCollider);
    void ForceRespawn();

private:
    Player* player = nullptr;

    Vector2D lastSafePosition;
    PhysBody* lastPlatformCollider = nullptr;
    int lastMovementDirection = 1;

    bool shouldRespawn = false;
};
