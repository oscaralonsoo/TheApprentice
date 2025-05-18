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
            jumpHoldTimer.Start();
            isJumping = true;
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
            jumpHoldTimer.Start();
            isJumping = true;
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

    if (isJumping && jumpHoldTimer.ReadMSec() > 0 && jumpRepeat) {
        float elapsedMs = (float)jumpHoldTimer.ReadMSec();
        float t = elapsedMs / jumpHoldDuration;

        // Clamp manual
        if (t < 0.0f) t = 0.0f;
        else if (t > 1.0f) t = 1.0f;

        float forceFactor = jumpHoldForceFactor * exp(-jumpDecayRate * t);
        float forceY = -progressiveJumpForce * forceFactor;

        b2Vec2 force(0, forceY);
        player->pbody->body->ApplyForceToCenter(force, true);

        // Si pasó el tiempo máximo, detenemos salto sostenido
        if (t >= 1.0f) {
            isJumping = false;
        }
    }

    // Finalizar salto si se suelta la tecla
    if (jumpUp || jumpHoldTimer.ReadMSec() > jumpHoldDuration) {
        isJumping = false;
    }

    // Planeo (solo después del doble salto y tras terminar el impulso)
    if (jumpCount == 2 && !isJumping &&
        glideUnlocked && jumpRepeat &&
        !player->GetMechanics()->IsOnGround() &&
        !player->GetMechanics()->IsWallSliding()) {

        if (!isGliding) {
            isGliding = true;
            player->pbody->body->SetGravityScale(glideGravityScale);
            player->SetState("idle");

            b2Vec2 vel = player->pbody->body->GetLinearVelocity();
            vel.y = 0.0f;
            player->pbody->body->SetLinearVelocity(vel);
        }

    }
    else if (isGliding && (!jumpRepeat || player->GetMechanics()->IsOnGround())) {
        isGliding = false;
        player->pbody->body->SetGravityScale(2.0f);
    }

    if (!isJumping &&
        !isGliding &&
        !player->GetMechanics()->IsOnGround() &&
        !player->GetMechanics()->IsWallSliding()) {

        b2Vec2 fallForce(0, fallAccelerationFactor);
        player->pbody->body->ApplyForceToCenter(fallForce, true);
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
    jumpCount = 0;
    player->GetMechanics()->SetIsOnGround(true);
    jumpCooldownActive = false;
    isGliding = false;
    player->pbody->body->SetGravityScale(2.0f);
}

void JumpMechanic::OnLeaveGround() {
    player->GetMechanics()->SetIsOnGround(false);
    jumpCooldownTimer.Start();
    jumpCooldownActive = true;
}

void JumpMechanic::SetController(SDL_GameController* controller) {
    this->controller = controller;
}

void JumpMechanic::EnableGlide(bool enable) {
    glideUnlocked = enable;
}