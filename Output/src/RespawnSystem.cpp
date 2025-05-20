#include "RespawnSystem.h"
#include "Player.h"
#include "Physics.h"
#include "Engine.h"
#include "Render.h"

void RespawnSystem::Init(Player* player) {
    this->player = player;
}

void RespawnSystem::Update(float dt) {
    if (shouldRespawn) {
        shouldRespawn = false;

        player->SetPosition(lastSafePosition);
        player->pbody->body->SetLinearVelocity(b2Vec2_zero);
        player->SetState("idle");

        Engine::GetInstance().render->StartCameraShake(0.5, 2);
    }
}

void RespawnSystem::UpdateLastSafePosition(PhysBody* platformCollider) {
    if (!platformCollider || !platformCollider->body) return;

    lastPlatformCollider = platformCollider;
    lastMovementDirection = player->GetMechanics()->GetMovementDirection();

    float width = platformCollider->width;
    float height = platformCollider->height;
    b2Vec2 posMeters = platformCollider->body->GetPosition();

    float topY = METERS_TO_PIXELS(posMeters.y) - (height / 2.0f);
    float verticalOffset = 100.0f;
    float respawnY = topY - verticalOffset;

    float platformCenterX = METERS_TO_PIXELS(posMeters.x) - 32;

    // Ahora se reaparece en el centro de la plataforma
    float respawnX = platformCenterX;

    lastSafePosition = Vector2D(respawnX, respawnY);
}

void RespawnSystem::ForceRespawn() {
    shouldRespawn = true;
}
