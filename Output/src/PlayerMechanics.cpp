#include "PlayerMechanics.h"
#include "Player.h"
#include "Physics.h"
#include "Scene.h"
#include "Log.h"

void PlayerMechanics::Init(Player* player) {
    this->player = player;

    movementHandler.Init(player);
    healthSystem.Init(player);
    invulnerabilitySystem.Init(player);
    godModeSystem.Init(player);
    respawnSystem.Init(player);
}

void PlayerMechanics::Update(float dt) {
    movementHandler.Update(dt);
    healthSystem.Update(dt);
    invulnerabilitySystem.Update(dt);
    godModeSystem.Update(dt);
    respawnSystem.Update(dt);
}

void PlayerMechanics::PostUpdate() {
    // De momento vacío
}

void PlayerMechanics::OnCollision(PhysBody* physA, PhysBody* physB) {
    movementHandler.OnCollision(physA, physB);

    // Y además, aquí podemos seguir llamando a HealthSystem, RespawnSystem, etc si toca.
    switch (physB->ctype) {
    case ColliderType::ENEMY: {
        b2Vec2 enemyPosMeters = physB->body->GetPosition();
        Vector2D enemyPos = Vector2D(METERS_TO_PIXELS(enemyPosMeters.x), METERS_TO_PIXELS(enemyPosMeters.y));

        // Aplicar knockback siempre
        healthSystem.ApplyKnockback(enemyPos);

        // Solo perder vida si no estás invulnerable ni en godmode
        if (!invulnerabilitySystem.IsInvulnerable() && !godModeSystem.IsEnabled()) {
            healthSystem.TakeDamage();
            invulnerabilitySystem.StartInvulnerability();
        }
        break;
    }
    case ColliderType::SPIKE:
        if (!godModeSystem.IsEnabled()) {
            healthSystem.HandleSpikeDamage();
            respawnSystem.ForceRespawn();
        }
        break;
    default:
        break;
    }
}

void PlayerMechanics::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    movementHandler.OnCollisionEnd(physA, physB);
}

void PlayerMechanics::EnableJump(bool enable) {
    movementHandler.EnableJump(enable);
}

void PlayerMechanics::HandleDash() {

    if (!dashUnlocked) return;

    if (isTouchingWall) return;

    if (!canDash && dashCooldown.ReadSec() >= dashMaxCoolDown) {
        canDash = true;
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_K) == KEY_DOWN && canDash) {
        isDashing = true;
        canDash = true;
        dashCooldown.Start();

        dashStartPosition = player->GetPosition();

        player->pbody->body->SetGravityScale(0.0f);

        // Direcci�n fija del dash
        dashDirection = movementDirection;

        if (isWallSliding) {
            dashDirection *= -1;
        }

        Engine::GetInstance().render->DashCameraImpulse(dashDirection, 100);

        if (attackSensor != nullptr) DestroyAttackSensor();
    }

    if (isDashing) {
        b2Vec2 vel(dashSpeed * dashDirection, 0.0f);
        player->pbody->body->SetLinearVelocity(vel);

        float distance = abs(player->GetPosition().getX() - dashStartPosition.getX());
        if (distance >= maxDashDistance) CancelDash();
    }
}

void PlayerMechanics::EnableDash(bool enable) {
    movementHandler.EnableDash(enable);
}

int PlayerMechanics::GetMovementDirection() const {
    return movementHandler.GetMovementDirection();
}

bool PlayerMechanics::IsVisible() const {
    return movementHandler.IsVisible() && invulnerabilitySystem.IsVisible();
}

void PlayerMechanics::ToggleGodMode() {
    godModeSystem.Toggle();
}

bool PlayerMechanics::IsGodMode() const {
    return godModeSystem.IsEnabled();
}

bool PlayerMechanics::IsOnGround() const {
    return isOnGround;
}

bool PlayerMechanics::IsWallSliding() const {
    return isWallSliding;
}

bool PlayerMechanics::IsTouchingWall() const {
    return isTouchingWall;
}

void PlayerMechanics::SetIsOnGround(bool isGrounded) {
    isOnGround = isGrounded;
}

void PlayerMechanics::SetIsTouchingWall(bool touching) {
    isTouchingWall = touching;
}
