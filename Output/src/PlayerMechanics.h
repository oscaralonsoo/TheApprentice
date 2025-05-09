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

    int GetWallSlideDirection() const { return wallSlideDirection; }
    void SetWallSlideDirection(int dir) { wallSlideDirection = dir; }

private:
    Player* player = nullptr;

    MovementHandler movementHandler;

    InvulnerabilitySystem invulnerabilitySystem;
    GodModeSystem godModeSystem;
    RespawnSystem respawnSystem;
    FallMechanic fallMechanic;

    bool isOnGround = false;
    int movementDirection = 1;
    bool isFalling = false;

    // Jump 
    float jumpForce = 13.0f;
    bool isJumping = false;
    bool jumpUnlocked = true;
    Timer jumpCooldownTimer;
    float jumpCooldownTime = 100.0f;
    bool jumpCooldownActive = false;
    float fallAccelerationFactor = 0.6f; // controla qu� tan r�pido acelera al caer tras soltar salto

    // Salto progresivo por altura
    float jumpStartY = 0.0f;
    float maxJumpHeight = 240.0f;     // altura m�xima del salto prolongado (en p�xeles)
    float minHoldJumpHeight = 20.0f; // altura m�nima para que empiece el hold jump
    float jumpHoldForceFactor = 0.85f; // fuerza inicial del hold jump
    bool isHoldingJump = false;
    float jumpDecayRate = 4.0f; // m�s alto = menos duraci�n de salto prolongado

    //Double Jump
    bool doubleJumpUnlocked = true;
    int jumpCount = 0;
    const int maxJumpCount = 2; // salto normal + doble salto

    // Dash
    bool isDashing = false;
    bool canDash = true;
    float dashSpeed = 15.0f;
    Vector2D dashStartPosition;
    float maxDashDistance = 100.0f;
    Timer dashCooldown;
    float dashMaxCoolDown = 1.0f;
    bool dashUnlocked = true;
    int dashDirection = 1;

    // Fall
    bool isStunned = false;
    float stunDuration = 1.0f;
    Timer stunTimer;
    float fallStartY = 0.0f;
    float fallDistanceThreshold = 500.0f;
    float fallEndY;
    float fallDistance;

    // WallSlide 
    bool wasInDownCameraZone = false;
    bool isWallSliding = false;
    bool isTouchingWall = false;
    int wallSlideDirection = 0;
};
