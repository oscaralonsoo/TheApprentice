#pragma once

#include "JumpMechanic.h"
#include "DashMechanic.h"
#include "AttackMechanic.h"
#include "FallMechanic.h"
#include "WallSlideMechanic.h"
#include <SDL2/SDL_gamecontroller.h>

class Player;

class MovementHandler {
public:
    void Init(Player* player);
    void Update(float dt);
    ~MovementHandler();

    void EnableJump(bool enable);
    void EnableDoubleJump(bool enable);
    void EnableDash(bool enable);

    int GetMovementDirection() const;
    bool IsVisible() const;
    void OnWallCollision();

    void SetCantMove(bool cantMove);
    bool GetCantMove() const{ return cantMove; }
    bool CanAttack() const;
    void SetCanAttack(bool canAttack);

    void OnCollision(PhysBody* physA, PhysBody* physB);
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    bool IsJumpCooldownActive() const { return jumpCooldownActive; }
    bool IsWallSlideCooldownActive() const { return wallSlideCooldownActive; }

    bool IsJumpUnlocked() const { return jumpMechanic.IsJumpUnlocked(); }
    bool IsDoubleJumpUnlocked() const { return jumpMechanic.IsDoubleJumpUnlocked(); }
    bool IsDashUnlocked() const { return dashMechanic.IsDashUnlocked(); }

    SDL_GameController* GetController() const { return controller; }
    JumpMechanic& GetJumpMechanic() { return jumpMechanic; }

    int GetWallSlideDirection() const { return wallSlideDirection; }
    void SetWallSlideDirection(int dir) { wallSlideDirection = dir; }
    bool IsWallSliding() const { return isWallSliding; }
    AttackMechanic& GetAttackMechanic() { return attackMechanic; }
    bool IsHookUnlocked() const { return hookUnlocked; }
    void SetHookUnlocked(bool unlocked) { hookUnlocked = unlocked; }
    void EnableGlide(bool enable);
    bool IsGlideUnlocked() const { return jumpMechanic.IsGlideUnlocked(); }
    WallSlideMechanic* GetWallSlideMechanic() { return &wallSlideMechanic; }
    DashMechanic& GetDashMechanic() { return dashMechanic; }
    void EnableWallJump(bool enable);
    bool IsWallJumpUnlocked() const;

    bool wallSlideFlip = false;
    bool disableAbilities = false;

    bool canJump = true;

    void StartWallSlideCooldown();

    void EnablePush(bool enable);
    bool CanPush() const;

    bool pendingLandingCheck = false;
    Timer pendingLandingTimer;
    float pendingLandingTimeout = 500.0f;

private:
    void HandleMovementInput();
    void UpdateAnimation();
    void HandleTimers();
    void HandleWallSlide();

private:
    Player* player = nullptr;

    JumpMechanic jumpMechanic;
    DashMechanic dashMechanic;
    AttackMechanic attackMechanic;
    FallMechanic fallMechanic;
    WallSlideMechanic wallSlideMechanic;

    int movementDirection = 1;
    bool visible = true;

    bool cantMove = false;
    bool canAttack = true;

    float speed = 8.0f;
    Timer jumpCooldownTimer;
    Timer wallSlideCooldownTimer;

    bool jumpCooldownActive = false;
    bool wallSlideCooldownActive = false;

    // Configuración de tiempos
    float jumpCooldownTime = 1.0f; // ms
    float wallSlideCooldownTime = 100.0f; // ms

    bool isWallSliding = false;
    bool isJumping = false; // para poder desactivar salto cuando haces wall slide
    SDL_GameController* controller = nullptr;

    int wallSlideDirection = 0;

    Timer downCameraCooldownTimer;
    float downCameraCooldownTime = 100.0f;
    bool downCameraCooldownActive = false;

    Timer boxCooldownTimer;
    float boxCooldownTime = 100.0f;
    bool boxCooldownActive = false;

    Timer lianaCooldownTimer;
    float lianaCooldownTime = 100.0f;
    bool lianaCooldownActive = false;

    bool isOnLiana = false;
    float lianaCenterX = 0.0f;

    bool hookUnlocked = false;

    PhysBody* lastPlatformCollider = nullptr;

    bool canPushBoxes = false;

    bool wasTouchingPlatform = false;

    bool aPressedNow = false;
    bool aJustPressed = false;

    int soundWalkId = 0;
    int soundClimbId = 0;
    
    bool isLanding = false;
    bool isMoving = false;

    Uint32 lastWalkSoundTime = 0;
    const Uint32 walkSoundInterval = 500;

    Uint32 lastClimbSoundTime = 0;
    const Uint32 climbSoundInterval = 500;
};
