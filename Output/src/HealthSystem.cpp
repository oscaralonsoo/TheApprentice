#include "HealthSystem.h"
#include "Player.h"
#include "Engine.h"
#include "Render.h"
#include "Scene.h"
#include "Physics.h"

void HealthSystem::Init(Player* player) {
    this->player = player;
}

void HealthSystem::Update(float dt) {
    UpdateVignette();
    CheckDeath();

    if (knockbackActive && knockbackTimer.ReadMSec() >= knockbackDuration) {
        knockbackActive = false;
        player->GetMechanics()->GetMovementHandler()->SetCantMove(false);
    }
}

void HealthSystem::TakeDamage() {
    lives--;
    if (lives < 0) lives = 0;

    Engine::GetInstance().render->StartCameraShake(0.5, 1);

    UpdateVignette();
}

void HealthSystem::HandleSpikeDamage() {
    if (lives > 0) {
        lives--;
    }
    if (lives <= 0) {
        Engine::GetInstance().scene->isDead = true;
    }
}

void HealthSystem::HealFull() {
    lives = 3;
    vignetteSize = 300.0f;
}

void HealthSystem::UpdateVignette() {
    vignetteSize = 300.0f + (3 - lives) * 200.0f;
    if (vignetteSize > 900.0f) vignetteSize = 900.0f;
    if (vignetteSize < 300.0f) vignetteSize = 300.0f;
}

void HealthSystem::CheckDeath() {
    if (lives <= 0) {
        Engine::GetInstance().scene->isDead = true;
        lives = 3; // reiniciamos para próximo respawn
    }
}

int HealthSystem::GetLives() const {
    return lives;
}

float HealthSystem::GetVignetteSize() const {
    return vignetteSize;
}

void HealthSystem::AddLife() {
    if (lives < 3) {
        lives++;
    }
    UpdateVignette();
}

void HealthSystem::SetLives(int lives) {
    this->lives = lives;
    UpdateVignette();
}

void HealthSystem::SetVignetteSize(float size) {
    vignetteSize = size;
}

void HealthSystem::ApplyKnockback(const Vector2D& sourcePosition) {
    b2Vec2 playerPos = player->pbody->body->GetPosition();
    float direction = (playerPos.x < PIXEL_TO_METERS(sourcePosition.x)) ? -1.0f : 1.0f;

    float horizontalPower = 4.0f;
    float verticalPower = -8.0f;

    knockbackInitialVelocity = b2Vec2(direction * horizontalPower, verticalPower);
    player->pbody->body->SetLinearVelocity(knockbackInitialVelocity);

    player->GetMechanics()->GetMovementHandler()->SetCantMove(true); // ← bloquear movimiento

    knockbackTimer.Start();
    knockbackActive = true;
}
