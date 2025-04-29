#include "JumpMechanic.h"
#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Physics.h"

void JumpMechanic::Init(Player* player) {
    this->player = player;
}

void JumpMechanic::Update(float dt) {
    if (!jumpUnlocked)
        return;

    HandleJumpInput();
}

void JumpMechanic::HandleJumpInput() {
    if (!jumpUnlocked) return;

    b2Vec2 velocity = player->pbody->body->GetLinearVelocity();

    // Inicio del salto o doble salto
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
        if (player->GetMechanics()->IsOnGround() || (doubleJumpUnlocked && jumpCount < maxJumpCount)) {
            velocity.y = -jumpForce;
            isJumping = true;
            isHoldingJump = true;
            jumpStartY = player->GetPosition().getY();
            jumpCount++;
            player->GetMechanics()->SetIsOnGround(false);
            player->SetState("jump");

            jumpCooldownTimer.Start();
            jumpCooldownActive = true;
        }
    }

    // Mantener salto pulsado
    if (isHoldingJump && Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT) {
        float currentY = player->GetPosition().getY();
        float heightJumped = jumpStartY - currentY;

        if (heightJumped >= minHoldJumpHeight && heightJumped < maxJumpHeight) {
            float t = (heightJumped - minHoldJumpHeight) / (maxJumpHeight - minHoldJumpHeight);
            float forceFactor = jumpHoldForceFactor * exp(-jumpDecayRate * t);
            velocity.y += -jumpForce * forceFactor * 0.05f;
        }
        else if (heightJumped >= maxJumpHeight) {
            isHoldingJump = false;
        }
    }

    // Cancelar impulso si se suelta la tecla
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_UP) {
        isHoldingJump = false;
    }

    // Caída rápida al soltar salto
    if (isJumping && !isHoldingJump && !player->GetMechanics()->IsWallSliding()) {
        velocity.y += fallAccelerationFactor;
    }

    player->pbody->body->SetLinearVelocity(velocity);
}

void JumpMechanic::Enable(bool enable) {
    jumpUnlocked = enable;
}

void JumpMechanic::EnableDoubleJump(bool enable) {
    doubleJumpUnlocked = enable;
}

void JumpMechanic::OnLanding() {
    isJumping = false;
    player->GetMechanics()->SetIsOnGround(true);
    jumpCount = 0;
}

void JumpMechanic::OnLeaveGround() {
    player->GetMechanics()->SetIsOnGround(false);
    jumpCooldownTimer.Start();
    jumpCooldownActive = true;
}
