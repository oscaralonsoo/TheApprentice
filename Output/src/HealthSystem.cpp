#include "HealthSystem.h"
#include "Player.h"
#include "Engine.h"
#include "Render.h"
#include "Scene.h"
#include "Physics.h"
#include "Audio.h"

void HealthSystem::Init(Player* player) {
    this->player = player;

    soundDamageId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Slime/slime_damage.ogg", 1.0f);
    soundDeadId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Slime/slime_dead.ogg", 1.0f);
}

void HealthSystem::Update(float dt) {
    UpdateVignette();
    CheckDeath();

    if (knockbackActive && knockbackTimer.ReadMSec() >= knockbackDuration) {
        knockbackActive = false;
        player->GetMechanics()->GetMovementHandler()->SetCantMove(false);
    }

    if (isInHitAnim && player->GetAnimation()->GetCurrentState() == "hit" && player->GetAnimation()->HasFinished()) {
        isInHitAnim = false;
    }

    if (isDying && player->GetAnimation()->GetCurrentState() == "die" && player->GetAnimation()->HasFinished()) {
        isDying = false;
        lives = maxlives;
        if (Engine::GetInstance().render->cameraLocked) {
            Engine::GetInstance().render->ToggleCameraLock();
        }
    }
}

void HealthSystem::TakeDamage() {
    Engine::GetInstance().audio->PlayFx(soundDamageId, 1.0f, 0);
    lives--;
    if (lives < 0) lives = 0;

    Engine::GetInstance().render->StartCameraShake(0.5, 1);

    if (lives <= 0 && !isDying) {
        Engine::GetInstance().audio->PlayFx(soundDeadId, 1.0f, 0);
        isDying = true;
        player->GetAnimation()->SetStateIfHigherPriority("die");
        Engine::GetInstance().scene->isDead = true;
    }
    else {
        player->GetAnimation()->SetStateIfHigherPriority("hit");
        isInHitAnim = true;
        Engine::GetInstance().render->StartCameraShake(0.5f, 1);
    }
}

void HealthSystem::HandleSpikeDamage() {
    if (lives > 0) {
        lives--;
    }

    if (lives <= 0 && !isDying) {
        isDying = true;
        player->GetAnimation()->SetStateIfHigherPriority("die");
        deathTimer.Start();
        // TODO JAVI --- SOLO HACE ESTOC CUANDO DEAD ANIM HA ACABADO
        Engine::GetInstance().scene->isDead = true;
    }
    else {
        player->GetAnimation()->SetStateIfHigherPriority("hit");
        hitTimer.Start();
        isInHitAnim = true;
    }
}

void HealthSystem::HealFull() {
    if (lives < 3)
    {
        lives = maxlives;
    }
    vignetteSize = 300.0f;
}

void HealthSystem::UpdateVignette() {
    vignetteSize = 300.0f + (3 - lives) * 200.0f;
    if (vignetteSize > 900.0f) vignetteSize = 900.0f;
    if (vignetteSize < 300.0f) vignetteSize = 300.0f;
}

void HealthSystem::CheckDeath() {
    if (GetLives() <= 0 && !isDying) {
        isDying = true;
        player->SetState("die");
        deathTimer.Start();

        Engine::GetInstance().scene->isDead = true;
        if (!Engine::GetInstance().scene->isChangingScene) {
            Engine::GetInstance().scene->isChangingScene = true; 
        }
    }
}


int HealthSystem::GetLives() const {
    return lives;
}

int HealthSystem::GetMaxLives() const {
    return maxlives;
}


float HealthSystem::GetVignetteSize() const {
    return vignetteSize;
}

void HealthSystem::AddLife() {
    if (lives < maxlives) {
        lives++;
        UpdateVignette();
    }
}

void HealthSystem::AddMaxLife() {
    maxlives++;
    lives++;
    UpdateVignette();
}

void HealthSystem::SetLives(int lives) {
    if (lives <= 0 || lives > maxlives) {
        this->lives = maxlives;
    }
    else {
        this->lives = lives;
    }
    UpdateVignette();
}

void HealthSystem::SetMaxLives(int lives) {
    this->maxlives = lives;
    UpdateVignette();
}

void HealthSystem::SetVignetteSize(float size) {
    vignetteSize = size;
}

void HealthSystem::ApplyKnockback(const Vector2D& sourcePosition) {
    if (lives <= 1) return;

    b2Vec2 playerPos = player->pbody->body->GetPosition();
    float direction = (playerPos.x < PIXEL_TO_METERS(sourcePosition.x)) ? -1.0f : 1.0f;

    float horizontalPower = 4.0f;
    float verticalPower = -8.0f;

    knockbackInitialVelocity = b2Vec2(direction * horizontalPower, verticalPower);
    player->pbody->body->SetLinearVelocity(knockbackInitialVelocity);

    player->GetMechanics()->GetMovementHandler()->SetCantMove(true);

    knockbackTimer.Start();
    knockbackActive = true;
}