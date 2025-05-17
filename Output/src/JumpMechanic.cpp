#include "JumpMechanic.h"
#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Physics.h"

void JumpMechanic::Init(Player* player) {
    this->player = player;
}

void JumpMechanic::Update(float dt) {
    if (!jumpUnlocked) return;
    HandleJumpInput(dt);
}

void JumpMechanic::HandleJumpInput(float dt) {
    if (!jumpUnlocked) return;

    // Entrada teclado
    std::shared_ptr<Input> input = Engine::GetInstance().input;
    bool spaceNow = input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT || input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN;

    bool jumpDown = false, jumpRepeat = false, jumpUp = false;

    if (spaceNow && !keyboardHeldPreviously) jumpDown = true;
    if (spaceNow) jumpRepeat = true;
    if (!spaceNow && keyboardHeldPreviously) jumpUp = true;

    keyboardHeldPreviously = spaceNow;

    // Entrada mando
    if (controller && SDL_GameControllerGetAttached(controller)) {
        bool controllerPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) != 0;

        if (controllerPressed && !controllerHeldPreviously) jumpDown = true;
        if (controllerPressed) jumpRepeat = true;
        if (!controllerPressed && controllerHeldPreviously) jumpUp = true;

        controllerHeldPreviously = controllerPressed;
    }

    // Iniciar salto
    if (jumpDown) {
        if (jumpCount == 0 && player->GetMechanics()->IsOnGround()) {
            isJumping = true;
            isHoldingJump = true;
            jumpStartY = player->GetPosition().getY();
            jumpCount = 1;
            player->GetMechanics()->SetIsOnGround(false);
            player->SetState("jump");

            jumpCooldownTimer.Start();
            jumpCooldownActive = true;

            // Impulso inicial para garantizar altura mínima
            b2Vec2 impulse(0, -minJumpForce);
            player->pbody->body->ApplyForceToCenter(impulse, true);
        }
        else if (doubleJumpUnlocked && jumpCount < maxJumpCount) {
            isJumping = true;
            isHoldingJump = true;
            jumpStartY = player->GetPosition().getY();
            jumpCount = 2;
            player->SetState("jump");

            jumpCooldownTimer.Start();
            jumpCooldownActive = true;

            // Resetear velocidad vertical para evitar acumulación
            b2Vec2 vel = player->pbody->body->GetLinearVelocity();
            vel.y = 0;
            player->pbody->body->SetLinearVelocity(vel);

            b2Vec2 impulse(0, -minJumpForce);
            player->pbody->body->ApplyForceToCenter(impulse, true);
        }
    }

    // Salto prolongado mientras se mantiene la tecla
    if (isHoldingJump && jumpRepeat) {
        float currentY = player->GetPosition().getY();
        float heightJumped = jumpStartY - currentY;

        if (heightJumped < maxJumpHeight) {
            float t = heightJumped / maxJumpHeight;
            float forceFactor = jumpHoldForceFactor * exp(-jumpDecayRate * t);
            float forceY = -progressiveJumpForce * forceFactor;

            b2Vec2 force(0, forceY);
            player->pbody->body->ApplyForceToCenter(force, true);
        }
        else {
            isHoldingJump = false;
        }
    }

    // Finalizar salto si se suelta la tecla
    if (jumpUp) {
        isHoldingJump = false;
    }

    // Aceleración de caída si no se está saltando ni deslizando por pared
    if (isJumping && !isHoldingJump && !player->GetMechanics()->IsWallSliding()) {
        float fallForce = fallAccelerationFactor;
        b2Vec2 force(0, fallForce);
        player->pbody->body->ApplyForceToCenter(force, true);
    }
}


void JumpMechanic::Enable(bool enable) {
    jumpUnlocked = enable;
}

void JumpMechanic::EnableDoubleJump(bool enable) {
    doubleJumpUnlocked = enable;
}

void JumpMechanic::OnLanding() {
    isJumping = false;
    isHoldingJump = false;
    jumpCount = 0;
    player->GetMechanics()->SetIsOnGround(true);
    jumpCooldownActive = false;
}

void JumpMechanic::OnLeaveGround() {
    player->GetMechanics()->SetIsOnGround(false);
    jumpCooldownTimer.Start();
    jumpCooldownActive = true;
}

void JumpMechanic::SetController(SDL_GameController* controller) {
    this->controller = controller;
}
