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
    void PostUpdate();

    void OnCollision(PhysBody* physA, PhysBody* physB);
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void EnableJump(bool enable) { jumpUnlocked = enable; }
    void EnableDoubleJump(bool enable) { doubleJumpUnlocked = enable; }
    void EnableDash(bool enable) { dashUnlocked = enable; }
    int GetMovementDirection() const { return movementDirection; }
    bool IsVisible() const { return visible; }
    void ToggleGodMode() { godMode = !godMode; }
    bool IsGodMode() const { return godMode; }
    void ChangeVignetteSize();

    float vignetteSize = 300.0f;
    bool cantMove = false;
    bool canAttack = true;
    int lives = 3;
    Vector2D lastPosition;

    bool doubleJumpUnlocked = false;
    bool jumpUnlocked;
    bool dashUnlocked = false;
private:
    void HandleInput();
    void HandleJump();
    void HandleDash();
    void HandleFall();
    void HandleWallSlide();
    void CancelDash();
    void CreateAttackSensor();
    void DestroyAttackSensor();
    void StartInvulnerability();
    void UpdateLastSafePosition();
    void HandleSound();
    void HandleGodMode();
    void HandleLives();

private:
    Player* player = nullptr;

    // Par�metros del jugador
    float speed = 8.0f;

    // F�sica
    bool isOnGround = false;
    int movementDirection = 1;
    bool isFalling = false;

    // Jump 
    float jumpForce = 13.0f;
    bool isJumping = false;;

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
    bool hasDoubleJump = false;
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

    int dashDirection = 1;

    // Fall
    bool isStunned = false;
    Timer stunTimer;
    float stunDuration = 910.0f; // en milisegundos
    float fallStunThreshold = 35.0f; // velocidad Y m�nima para provocar stun
    bool willStun = false;
    Timer landingToIdleTimer;
    bool waitingToIdle = false;
    float landingToIdleDelay = 200.0f;

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
    float attackDuration = 500.0f;
    int playerAttackX;
    int playerAttackY;
    bool isAttacking = false;
    Timer attackDelayTimer;
    bool attackPending = false;

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
    bool isDying = false;

    //Down Camera
    int originalCameraOffsetY;
    bool inDownCameraZone = false;
    Timer downCameraCooldown;
    float downCameraCooldownTime = 100.0f;
    bool cameraModifiedByZone = false;

    //God mode
    bool godMode = false;

    //Sounds
    int slimeFxId = -1;
    int slimeChannel = -1;
    int jumpFxId = -1;
    bool isSlimeSoundPlaying = false;
    bool playJumpSound = false;

    //Enemies
    bool knockbackActive = false;
    Timer knockbackTimer;
    float knockbackDuration = 300.0f;
    b2Vec2 knockbackInitialVelocity = { 0, 0 };
    float knockbackProgress = 0.0f;
    float knockbackTotalTime = 300.0f; // en milisegundos�

    //Spikes
    Timer spikesCouldown;
    float maxTimeSpikesCouldown = 100.0f;
    bool spikesDamage = false;
};
