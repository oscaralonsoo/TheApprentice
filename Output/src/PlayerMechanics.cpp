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

        // Solo perder vida si no estás invulnerable ni en godmode
        if (!invulnerabilitySystem.IsInvulnerable() && !godModeSystem.IsEnabled()) {
            healthSystem.TakeDamage();
            invulnerabilitySystem.StartInvulnerability();
            healthSystem.ApplyKnockback(enemyPos);
        }
        break;
    }
    case ColliderType::SPIKE:
        if (!godModeSystem.IsEnabled() && !invulnerabilitySystem.IsInvulnerable()) {
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

void PlayerMechanics::EnableDoubleJump(bool enable) {
    movementHandler.EnableDoubleJump(enable);
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

void PlayerMechanics::EnableGlide(bool enable) {
    movementHandler.EnableGlide(enable);
}

void PlayerMechanics::EnableWallJump(bool enable) {
    movementHandler.EnableWallJump(enable);
}

void PlayerMechanics::UpdateLastSafePosition(PhysBody* platformCollider) {
    respawnSystem.UpdateLastSafePosition(platformCollider);
}

void PlayerMechanics::EnablePush(bool enable) {
    movementHandler.EnablePush(enable);
}

bool PlayerMechanics::CanPush() const {
    return movementHandler.CanPush();
}