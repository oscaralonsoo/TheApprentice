#include "FallMechanic.h"
#include "Player.h"
#include "Engine.h"
#include "Physics.h"
#include "Render.h"

void FallMechanic::Init(Player* player) {
    this->player = player;
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
        player->GetAnimation()->SetStateIfHigherPriority("fall");
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
            player->SetState("landing_stun");
            Engine::GetInstance().render->StartCameraShake(0.2f, 2);
        }
        else {
            player->SetState("landing");
        }
    }
}

void FallMechanic::CheckFallStart() {
    b2Vec2 velocity = player->pbody->body->GetLinearVelocity();

    if (velocity.y > 0.1f && !isFalling) {

        isFalling = true;
        player->SetState("fall");
    }
}

void FallMechanic::OnLanding() {
    if (isFalling) {
        isFalling = false;

        float verticalVelocity = player->pbody->body->GetLinearVelocity().y;

        if (verticalVelocity > fallStunThreshold) {
            isStunned = true;
            stunTimer.Start();
            player->SetState("landing_stun");
            Engine::GetInstance().render->StartCameraShake(0.2f, 2);
        }
        else {
            player->SetState("landing");
        }
    }
}