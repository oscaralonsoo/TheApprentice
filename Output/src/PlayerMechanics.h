// PlayerMechanics.h
#pragma once

#include "Vector2D.h"
#include "Timer.h"
#include "Physics.h"

class Player;
class PhysBody;

class PlayerMechanics {
public:
    void Init(Player* player);
    void Update(float dt);

    void OnCollision(PhysBody* physA, PhysBody* physB);
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void EnableJump(bool enable) { jumpUnlocked = enable; }
    void EnableDoubleJump(bool enable) { doubleJumpUnlocked = enable; }
    void EnableDash(bool enable) { dashUnlocked = enable; }
    int GetMovementDirection() const { return movementDirection; }
    bool IsVisible() const { return visible; }


    bool cantMove = false;
    bool canAttack = true;


private:
    void HandleInput();
    void HandleJump();
    void HandleDash();
    void HandleFall();
    void CheckFallImpact();
    void HandleWallSlide();
    void CancelDash();
    void CreateAttackSensor();
    void DestroyAttackSensor();
    void StartInvulnerability();
    void UpdateLastSafePosition();

private:
    Player* player = nullptr;

    // Parámetros del jugador
    float speed = 8.0f;
    int vidas = 3;

    // Física
    bool isOnGround = false;
    int movementDirection = 1;
    bool isFalling = false;

    // Jump
    float jumpForce = 10.5f;
    bool isJumping = false;
    float jumpTime = 0.0f;
    float maxJumpTime = 0.3f;
    bool jumpUnlocked = true;

    // DoubleJump
    bool doubleJumpUnlocked = true;
    bool hasDoubleJumped = false;

    // Dash
    bool isDashing = false;
    bool canDash = true;
    float dashSpeed = 15.0f;
    Vector2D dashStartPosition;
    float maxDashDistance = 100.0f;
    Timer dashCooldown;
    float dashMaxCoolDown = 1.0f;
    bool dashUnlocked = true;

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
    bool wallJumpUnlocked = true;
    Timer wallSlideCooldownTimer;
    float wallSlideCooldownTime = 0.15f; // en segundos, ajustable
    bool wallSlideCooldownActive = false;

    // Attack
    PhysBody* attackSensor = nullptr;
    Timer attackTimer;
    float attackDuration = 200.0f;
    int playerAttackX;
    int playerAttackY;
    bool isAttacking = false;

    bool isInvulnerable = false;
    Timer invulnerabilityTimer;
    float invulnerabilityDuration = 3.0f;
    bool visible = true;
    Timer blinkTimer;
    float blinkInterval = 150.0f;

    Vector2D lastPosition;
    bool shouldRespawn = false;
    PhysBody* lastPlatformCollider = nullptr;
    Vector2D respawnPosition;
    int lasMovementDirection = 1;

};
