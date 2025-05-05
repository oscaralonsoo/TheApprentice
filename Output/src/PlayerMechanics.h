#pragma once

#include "MovementHandler.h"
#include "HealthSystem.h"
#include "InvulnerabilitySystem.h"
#include "GodModeSystem.h"
#include "RespawnSystem.h"
#include "FallMechanic.h"

class Player;
class PhysBody;

class PlayerMechanics {
public:
    void Init(Player* player);
    void Update(float dt);
    void PostUpdate();

    void OnCollision(PhysBody* physA, PhysBody* physB);
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void EnableJump(bool enable);
    void EnableDoubleJump(bool enable);
    void EnableDash(bool enable);

    int GetMovementDirection() const;
    bool IsVisible() const;
    void ToggleGodMode();
    bool IsGodMode() const;
    bool IsOnGround() const;
    bool IsWallSliding() const;
    bool IsTouchingWall() const;
    void SetIsTouchingWall(bool touching);
    void SetIsOnGround(bool isGrounded);

    MovementHandler* GetMovementHandler() { return &movementHandler; }
    HealthSystem* GetHealthSystem() { return &healthSystem; }
    FallMechanic* GetFallMechanic() { return &fallMechanic; }

    HealthSystem healthSystem;
private:
    Player* player = nullptr;

    MovementHandler movementHandler;

    InvulnerabilitySystem invulnerabilitySystem;
    GodModeSystem godModeSystem;
    RespawnSystem respawnSystem;
    FallMechanic fallMechanic;

    bool isOnGround = false;
    bool isWallSliding = false;
    bool isTouchingWall = false;
};
