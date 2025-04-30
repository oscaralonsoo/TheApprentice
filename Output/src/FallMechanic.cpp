#include "FallMechanic.h"
#include "Player.h"
#include "Engine.h"
#include "Physics.h"
#include "Render.h"

void FallMechanic::Init(Player* player) {
    this->player = player;
}

void FallMechanic::Update(float dt) {
    if (player->GetMechanics()->IsOnGround())
        return;

    if (isStunned) {
        if (stunTimer.ReadMSec() >= stunDuration) {
            isStunned = false;
            player->SetState("idle");
        }
        else {
            player->pbody->body->SetLinearVelocity(b2Vec2_zero);
            player->SetState("landing");
        }
        return;
    }

    b2Vec2 velocity = player->pbody->body->GetLinearVelocity();
    if (velocity.y > 0.1f && !isFalling && !player->GetMechanics()->IsWallSliding()) {
        isFalling = true;
        player->SetState("fall"); //  aqu� seteamos animaci�n de ca�da
    }

    CheckFallStart();
    ApplyFallStunIfNeeded();
    CheckLanding();
}

void FallMechanic::CheckFallStart() {
    b2Vec2 velocity = player->pbody->body->GetLinearVelocity();

    if (velocity.y > 0.1f && !isFalling) {
        isFalling = true;
        player->SetState("fall");
    }
}

void FallMechanic::ApplyFallStunIfNeeded() {
    float verticalVelocity = player->pbody->body->GetLinearVelocity().y;

    if (verticalVelocity > fallStunThreshold) {
        willStun = true;
    }
}

void FallMechanic::CheckLanding() {
    if (player->GetMechanics()->IsOnGround() && isFalling) {
        isFalling = false;

        if (willStun) {
            willStun = false;
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

void FallMechanic::OnLanding() {
    if (isFalling) {
        isFalling = false;

        if (willStun) {
            willStun = false;
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