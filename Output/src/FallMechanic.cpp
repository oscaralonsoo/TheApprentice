#include "FallMechanic.h"
#include "Player.h"
#include "Engine.h"
#include "Physics.h"
#include "Render.h"
#include "Audio.h"

void FallMechanic::Init(Player* player) {
    this->player = player;

    soundStunId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Slime/slime_stun.ogg", 1.0f);
}

void FallMechanic::Update(float dt) {
    if (isStunned) {
        if (stunTimer.ReadMSec() >= stunDuration) {
            isStunned = false;
            player->GetAnimation()->ForceSetState("idle");
        }
        else {
            player->pbody->body->SetLinearVelocity(b2Vec2_zero);
            player->GetAnimation()->SetStateIfHigherPriority("landing_stun");
        }
        return;
    }

    if (player->GetMechanics()->IsOnGround()) {
        return;
    }

    b2Vec2 velocity = player->pbody->body->GetLinearVelocity();
    if (velocity.y > 0.1f && !isFalling) {
        isFalling = true;
        player->GetAnimation()->ForceSetState("fall");
    }

    CheckFallStart();
    CheckLanding();
}

void FallMechanic::CheckLanding() {
    if (player->GetMechanics()->IsOnGround() && isFalling) {
        isFalling = false;
        float verticalVelocity = player->pbody->body->GetLinearVelocity().y;

        if (verticalVelocity > fallStunThreshold) {
            isStunned = true;
            stunTimer.Start();
            Engine::GetInstance().render->StartCameraShake(0.2f, 2);
        }
    }
}

void FallMechanic::CheckFallStart() {
    b2Vec2 velocity = player->pbody->body->GetLinearVelocity();

    if (velocity.y > 0.1f && !isFalling) {

        isFalling = true;
    }
}

void FallMechanic::OnLanding() {
    if (isFalling) {
        isFalling = false;

        float verticalVelocity = player->pbody->body->GetLinearVelocity().y;

        if (verticalVelocity > fallStunThreshold) {
            isStunned = true;
            Engine::GetInstance().audio->PlayFx(soundStunId, 1.0f, 0);
            stunTimer.Start();
            Engine::GetInstance().render->StartCameraShake(0.2f, 2);
        }
        if (!player->GetMechanics()->GetMovementHandler()->GetAttackMechanic().IsAttacking())
        {
            player->GetAnimation()->ForceSetState("idle");
        }
    }
}