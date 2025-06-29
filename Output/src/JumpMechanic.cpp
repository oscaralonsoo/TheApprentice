#include "JumpMechanic.h"
#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Physics.h"
#include "Log.h"
#include "Audio.h"

void JumpMechanic::Init(Player* player) {
    this->player = player;
    soundJumpId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Slime/slime_jump.ogg", 1.0f);
}

void JumpMechanic::Update(float dt) {
    if (!jumpUnlocked) return;
    HandleJumpInput(dt);
    // Desbloquear input horizontal tras wall jump
    if (wallJumpLockActive && wallJumpLockTimer.ReadMSec() >= wallJumpLockDuration) {
        wallJumpLockActive = false;
    }
}

void JumpMechanic::HandleJumpInput(float dt) {
    if (!jumpUnlocked) return;

    if (player->GetMechanics()->GetMovementHandler()->IsDashUnlocked() &&
        player->GetMechanics()->GetMovementHandler()->GetDashMechanic().IsDashing()) {
        return;
    }

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
            if (!jumpSoundPlayed) {
                Engine::GetInstance().audio->PlayFx(soundJumpId, 1.0f, 0);
                jumpSoundPlayed = true;
            }
            jumpCount = 1;
            player->GetMechanics()->SetIsOnGround(false);
            player->GetAnimation()->SetOverlayState("transition");
            player->GetAnimation()->SetStateIfHigherPriority("jump");
            jumpInterrupted = false;

            jumpCooldownTimer.Start();
            jumpCooldownActive = true;

            // Impulso inicial para garantizar altura m�nima
            b2Vec2 impulse(0, -minJumpForce);
            player->pbody->body->ApplyForceToCenter(impulse, true);
        }
        else if (doubleJumpUnlocked && jumpCount < maxJumpCount && !player->GetMechanics()->IsWallSliding()) {
            jumpHoldTimer.Start();
            isJumping = true;
            jumpInterrupted = false;
            jumpCount = 2;
            player->GetAnimation()->SetOverlayState("transition");
            player->GetAnimation()->SetStateIfHigherPriority("doublejump");
            Engine::GetInstance().audio->PlayFx(soundJumpId, 1.0f, 0);
            jumpCooldownTimer.Start();
            jumpCooldownActive = true;

            // Resetear velocidad vertical para evitar acumulaci�n
            b2Vec2 vel = player->pbody->body->GetLinearVelocity();
            vel.y = 0;
            player->pbody->body->SetLinearVelocity(vel);

            b2Vec2 impulse(0, -minJumpForce);
            player->pbody->body->ApplyForceToCenter(impulse, true);
        }
        else if (glideUnlocked && jumpCount == 2 && !player->GetMechanics()->IsOnGround() && !player->GetMechanics()->IsWallSliding()) {
            jumpCount = 3;
            isGliding = true;
            player->pbody->body->SetGravityScale(glideGravityScale);
            player->GetAnimation()->SetOverlayState("transition");
            player->GetAnimation()->SetStateIfHigherPriority("glide");

            b2Vec2 vel = player->pbody->body->GetLinearVelocity();
            vel.y = 0.0f;
            player->pbody->body->SetLinearVelocity(vel);
        }
        if (jumpCount == 0 && wallJumpUnlocked && player->GetMechanics()->IsWallSliding()) {
            isJumping = true;
            wallJumpActive = true;
            jumpCount = 1;

            player->GetAnimation()->SetOverlayState("transition");
            player->GetAnimation()->SetStateIfHigherPriority("walljump");
            player->GetMechanics()->SetIsWallSliding(false);
            player->GetMechanics()->SetIsTouchingWall(false);

            MovementHandler* movement = player->GetMechanics()->GetMovementHandler();
            int wallDir = movement->GetWallSlideDirection();
            movement->StartWallSlideCooldown();
            movement->SetWallSlideDirection(0);

            jumpCooldownTimer.Start();
            jumpCooldownActive = true;

            // Impulso fuerte
            player->pbody->body->SetLinearVelocity(b2Vec2_zero);
            b2Vec2 impulse(-wallDir * wallJumpHorizontalImpulse, -wallJumpVerticalImpulse);
            player->pbody->body->ApplyLinearImpulseToCenter(impulse, true);

            // Bloqueo de input horizontal
            wallJumpLockActive = true;
            wallJumpLockTimer.Start();
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

        if (t >= 1.0f) {
            isJumping = false;

            // Solo la primera vez que se interrumpe el salto, seteamos la velocidad Y a 0
            if (!jumpInterrupted) {
                b2Vec2 vel = player->pbody->body->GetLinearVelocity();
                vel.y = 0;
                player->pbody->body->SetLinearVelocity(vel);
                jumpInterrupted = true;
            }
        }
    }

    if ((jumpUp || jumpHoldTimer.ReadMSec() > jumpHoldDuration) && isJumping) {
        isJumping = false;

        if (!jumpInterrupted) {
            b2Vec2 vel = player->pbody->body->GetLinearVelocity();
            vel.y = 3;
            player->pbody->body->SetLinearVelocity(vel);
            jumpInterrupted = true;
        }
    }

    // Planeo (solo despu�s del doble salto y tras terminar el impulso)
    if (jumpDown && jumpCount == 2 && !isJumping &&
        glideUnlocked &&
        !player->GetMechanics()->IsOnGround() &&
        !player->GetMechanics()->IsWallSliding()) {

        glideInputArmed = true; // Estamos listos para iniciar planeo si se mantiene pulsado
    }

    // Activar el planeo solo si está armado y se mantiene pulsado
    if (glideInputArmed && jumpRepeat &&
        !player->GetMechanics()->IsOnGround() &&
        !player->GetMechanics()->IsWallSliding()) {

        if (!isGliding) {
            isGliding = true;
            player->pbody->body->SetGravityScale(glideGravityScale);
            player->GetAnimation()->SetOverlayState("transition");
            player->GetAnimation()->SetStateIfHigherPriority("glide");

            b2Vec2 vel = player->pbody->body->GetLinearVelocity();
            vel.y = 0.0f;
            player->pbody->body->SetLinearVelocity(vel);
        }
    }
    else if (isGliding && (!jumpRepeat || player->GetMechanics()->IsOnGround() || jumpCount < 3)) {
        isGliding = false;
        player->pbody->body->SetGravityScale(2.0f);

        if (!player->GetMechanics()->IsOnGround() && !player->GetMechanics()->IsWallSliding()) {
            player->GetAnimation()->ForceSetState("fall");
        }
        glideInputArmed = false; // Resetear el estado
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
    wallJumpActive = false;
    glideInputArmed = false;
    jumpSoundPlayed = false;
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

void JumpMechanic::EnableWallJump(bool enable) {
    wallJumpUnlocked = enable;
}