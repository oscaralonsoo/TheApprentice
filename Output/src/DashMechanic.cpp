#include "DashMechanic.h"
#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Physics.h"
#include "Render.h"

void DashMechanic::Init(Player* player) {
    this->player = player;
}

void DashMechanic::Update(float dt) {
    if (!dashUnlocked)
        return;
    // Bloquear dash si estás en wallslide o tocando una pared
    if (player->GetMechanics()->GetMovementHandler()->IsWallSliding() ||
        player->GetMechanics()->IsTouchingWall()) {
        return;
    }

    if (!canDash && dashCooldownTimer.ReadSec() >= dashCooldownTime) {
        canDash = true;
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
    player->SetState("dash");
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
