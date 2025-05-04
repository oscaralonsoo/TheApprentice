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

    // Estados de entrada general
    bool jumpDown = false;
    bool jumpRepeat = false;
    bool jumpUp = false;

    // ----------- TECLADO -----------
    std::shared_ptr<Input> input = Engine::GetInstance().input;
    bool spaceNow = input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT || input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN;

    if (spaceNow && !keyboardHeldPreviously) {
        jumpDown = true;
    }
    if (spaceNow) {
        jumpRepeat = true;
    }
    if (!spaceNow && keyboardHeldPreviously) {
        jumpUp = true;
    }

    keyboardHeldPreviously = spaceNow;

    // ----------- MANDO -----------
    if (controller && SDL_GameControllerGetAttached(controller)) {
        bool controllerPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) != 0;

        if (controllerPressed && !controllerHeldPreviously) {
            jumpDown = true;
        }
        if (controllerPressed) {
            jumpRepeat = true;
        }
        if (!controllerPressed && controllerHeldPreviously) {
            jumpUp = true;
        }

        controllerHeldPreviously = controllerPressed;
    }

    // ----------- INICIO DE SALTO -----------
    if (jumpDown && !player->GetMechanics()->GetFallMechanic()->IsFalling()) {
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

    // ----------- SALTO PROGRESIVO -----------
    if (isHoldingJump && jumpRepeat && !player->GetMechanics()->GetFallMechanic()->IsFalling()) {
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

    // ----------- FIN DEL SALTO -----------
    if (jumpUp) {
        isHoldingJump = false;
    }

    // ----------- CAÍDA ACELERADA -----------
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

void JumpMechanic::SetController(SDL_GameController* controller) {
    this->controller = controller;
}
