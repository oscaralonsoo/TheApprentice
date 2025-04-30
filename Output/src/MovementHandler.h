#pragma once

#include "JumpMechanic.h"
#include "DashMechanic.h"
#include "AttackMechanic.h"
#include "FallMechanic.h"
#include "WallSlideMechanic.h"

class Player;

class MovementHandler {
public:
    void Init(Player* player);
    void Update(float dt);

    void EnableJump(bool enable);
    void EnableDoubleJump(bool enable);
    void EnableDash(bool enable);

    int GetMovementDirection() const;
    bool IsVisible() const;
    void OnWallCollision();

    void SetCantMove(bool cantMove);
    bool CanAttack() const;
    void SetCanAttack(bool canAttack);

    void OnCollision(PhysBody* physA, PhysBody* physB);
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    bool IsJumpCooldownActive() const { return jumpCooldownActive; }
    bool IsWallSlideCooldownActive() const { return wallSlideCooldownActive; }

    bool IsJumpUnlocked() const { return jumpMechanic.IsJumpUnlocked(); }
    bool IsDoubleJumpUnlocked() const { return jumpMechanic.IsDoubleJumpUnlocked(); }
    bool IsDashUnlocked() const { return dashMechanic.IsDashUnlocked(); }

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
    float jumpCooldownTime = 100.0f; // ms
    float wallSlideCooldownTime = 100.0f; // ms

    bool isWallSliding = false;
    bool isJumping = false; // para poder desactivar salto cuando haces wall slide
};
