#include "DashMechanic.h"
#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Physics.h"
#include "Render.h"
#include "Audio.h"

void DashMechanic::Init(Player* player) {
    this->player = player;

    soundDashId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Slime/slime_dash.ogg", 1.0f);
}

void DashMechanic::Update(float dt) {
    if (!dashUnlocked)
        return;

    if (!canDash && dashCooldownTimer.ReadSec() >= dashCooldownTime) {
        canDash = true;
        player->GetAnimation()->ForceSetState("idle");
    }

    bool dashPressed = false;

    // Teclado (K)
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_K) == KEY_DOWN) {
        dashPressed = true;
    }

    // Mando (RT)
    if (controller && SDL_GameControllerGetAttached(controller)) {
        Sint16 triggerValue = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
        const Sint16 triggerThreshold = 16000;

        if (triggerValue > triggerThreshold && !rtHeldPreviously) {
            dashPressed = true;
        }

        rtHeldPreviously = (triggerValue > triggerThreshold);
    }

    if (dashPressed && canDash) {
        StartDash();
    }

    if (isDashing) {
        ApplyDashMovement();
    }
}

void DashMechanic::StartDash() {
    isDashing = true;
    canDash = false;
    dashCooldownTimer.Start();
    Engine::GetInstance().audio->PlayFx(soundDashId, 1.0f, 0);

    dashStartPosition = player->GetPosition();
    lastPosition = dashStartPosition;  // Inicializar control de atasco
    stuckFrameCounter = 0;

    player->pbody->body->SetGravityScale(0.0f);

    if (player->GetMechanics()->IsWallSliding()) {
        dashDirection = -player->GetMechanics()->GetMovementHandler()->GetWallSlideDirection();
        player->GetMechanics()->GetMovementHandler()->StartWallSlideCooldown();
        player->GetMechanics()->SetIsTouchingWall(false);
    }
    else {
        dashDirection = player->GetMechanics()->GetMovementDirection();
    }

    Engine::GetInstance().render->DashCameraImpulse(dashDirection, 100);
    player->GetAnimation()->SetOverlayState("transition");
    player->GetAnimation()->SetStateIfHigherPriority("dash");
}

void DashMechanic::ApplyDashMovement() {
    b2Vec2 velocity(dashSpeed * dashDirection, 0.0f);
    player->pbody->body->SetLinearVelocity(velocity);

    float distance = abs(player->GetPosition().getX() - dashStartPosition.getX());
    if (distance >= maxDashDistance) {
        CancelDash();
        return;
    }

    // Comprobar si el jugador se ha quedado atascado (sin moverse)
    Vector2D currentPosition = player->GetPosition();
    if (abs(currentPosition.getX() - lastPosition.getX()) < 0.1f) {  // Umbral mínimo de movimiento
        stuckFrameCounter++;
        if (stuckFrameCounter >= stuckFrameLimit) {
            CancelDash();
            return;
        }
    }
    else {
        stuckFrameCounter = 0;  // Resetear contador si se ha movido
    }

    lastPosition = currentPosition;
}

void DashMechanic::CancelDash() {
    isDashing = false;
    player->pbody->body->SetGravityScale(2.0f);

    b2Vec2 velocity = player->pbody->body->GetLinearVelocity();
    velocity.x = 0.0f;
    player->pbody->body->SetLinearVelocity(velocity);

    // Evitar reenganche inmediato al wall slide
    player->GetMechanics()->GetMovementHandler()->StartWallSlideCooldown();

    if (!player->GetMechanics()->IsOnGround()) {
        player->GetAnimation()->ForceSetState("fall");
    }
    else {
        player->GetAnimation()->ForceSetState("idle");
    }

}

void DashMechanic::OnWallCollision() {
    if (isDashing) {
        CancelDash();
    }
}

void DashMechanic::Enable(bool enable) {
    dashUnlocked = enable;
}

bool DashMechanic::IsDashing() const {
    return isDashing;
}

void DashMechanic::SetController(SDL_GameController* controller) {
    this->controller = controller;
}
