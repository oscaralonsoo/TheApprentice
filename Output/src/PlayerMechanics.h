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
    void ToggleGodMode() { godMode = !godMode; }
    bool IsGodMode() const { return godMode; }

    float vignetteSize = 300.0f;
    bool cantMove = false;
    bool canAttack = true;
    int vidas = 3;
    Vector2D lastPosition;

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
    void HandleSound();
    void HandleGodMode();
    void ReduceVignetteSize();
private:
    Player* player = nullptr;



    // Parámetros del jugador
    float speed = 8.0f;

    // Física
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
    float fallAccelerationFactor = 0.6f; // controla qué tan rápido acelera al caer tras soltar salto

    // Salto progresivo por altura
    float jumpStartY = 0.0f;
    float maxJumpHeight = 240.0f;     // altura máxima del salto prolongado (en píxeles)
    float minHoldJumpHeight = 20.0f; // altura mínima para que empiece el hold jump
    float jumpHoldForceFactor = 0.85f; // fuerza inicial del hold jump
    bool isHoldingJump = false;
    float jumpDecayRate = 4.0f; // más alto = menos duración de salto prolongado

    //Double Jump
    bool doubleJumpUnlocked = false;
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
    bool dashUnlocked = false;
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
    bool wallJumpUnlocked = true;
    Timer wallSlideCooldownTimer;
    float wallSlideCooldownTime = 100.0f;
    bool wallSlideCooldownActive = false;
    
    //Wall
    Timer wallCooldownTimer;
    float wallCooldownTime = 100.0f;
    bool wallCooldownActive = false;
    bool isTouchingWall = false;

    // Attack
    PhysBody* attackSensor = nullptr;
    Timer attackTimer;
    float attackDuration = 200.0f;
    int playerAttackX;
    int playerAttackY;
    bool isAttacking = false;

    //Invulnerable
    bool isInvulnerable = false;
    Timer invulnerabilityTimer;
    float invulnerabilityDuration = 3.0f;
    bool visible = true;
    Timer blinkTimer;
    float blinkInterval = 150.0f;

    //Respawn 
    bool shouldRespawn = false;
    PhysBody* lastPlatformCollider = nullptr;
    int lasMovementDirection = 1;

    //Down Camera
    int originalCameraOffsetY;
    bool inDownCameraZone = false;
    Timer downCameraCooldown;
    float downCameraCooldownTime = 0.2f;

    //God mode
    bool godMode = false;

    //Sounds
    int slimeFxId = -1;
    int slimeChannel = -1;
    int jumpFxId = -1;
    bool isSlimeSoundPlaying = false;
    bool playJumpSound = false;
};
