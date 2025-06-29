#include "MovementHandler.h"
#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Physics.h"
#include "AbilityZone.h"
#include "Log.h"
#include "EntityManager.h"
#include "HookAnchor.h"
#include "Scene.h"
#include "Audio.h"
#include <cmath>


void MovementHandler::Init(Player* player) {
    this->player = player;

    // Inicializar SDL GameController
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

    controller = nullptr;
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) break;
        }
    }

    // Inicializar las mec�nicas despu�s de preparar el controller
    jumpMechanic.Init(player);
    dashMechanic.Init(player);
    attackMechanic.Init(player);
    fallMechanic.Init(player);
    wallSlideMechanic.Init(player);

    if (controller) {
        jumpMechanic.SetController(controller);
        dashMechanic.SetController(controller);
        attackMechanic.SetController(controller);
        Engine::GetInstance().menus->SetController(controller);

        // Reasignar controller a todas las AbilityZones una vez inicializado
        for (Entity* e : Engine::GetInstance().entityManager->entities) {
            if (e->type == EntityType::ABILITY_ZONE) {
                static_cast<AbilityZone*>(e)->SetController(controller);
                LOG("Controller reasignado a AbilityZone desde MovementHandler");
            }
        }
    }

    soundWalkId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Slime/slime_walk.ogg", 1.0f);
    soundClimbId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Slime/slime_climbing.ogg", 1.0f);
}

void MovementHandler::Update(float dt) {
    fallMechanic.Update(dt);
    if (fallMechanic.IsStunned()) return;

    if (isOnLiana) {
        player->GetAnimation()->ForceSetState("liana");
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastClimbSoundTime >= climbSoundInterval) {
            Engine::GetInstance().audio->PlayFx(soundClimbId, 1.0f, 0);
            lastClimbSoundTime = currentTime;
        }

        b2Vec2 velocity(0.0f, 0.0f);

        // --------- Mando ---------
        if (controller && SDL_GameControllerGetAttached(controller)) {
            Sint16 axisY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
            Sint16 axisX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
            const Sint16 deadZone = 8000;

            if (axisY < -deadZone) velocity.y = -8.0f;
            else if (axisY > deadZone) velocity.y = 8.0f;

            if (axisX < -deadZone) velocity.x = -8.0f;
            else if (axisX > deadZone) velocity.x = 8.0f;
        }

        // --------- Teclado ---------
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
            velocity.y = -8.0f;
        }
        else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
            velocity.y = 8.0f;
        }

        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
            velocity.x = -8.0f;
        }
        else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
            velocity.x = 8.0f;
        }

        // --------- Centrarse en la cuerda si se mueve verticalmente ---------
        if (velocity.y != 0.0f) {
            float currentX = player->GetPosition().getX(); // PIXELES
            float distance = lianaCenterX - currentX;
            float centerSpeed = 1.0f * dt;
            float newX = currentX + centerSpeed * ((distance > 0) ? 1.0f : -1.0f);
            if (fabs(distance) < centerSpeed) newX = lianaCenterX;
            player->pbody->body->SetTransform(b2Vec2(PIXEL_TO_METERS(newX), player->pbody->body->GetPosition().y), 0.0f);
        }

        player->pbody->body->SetGravityScale(0.0f);
        player->pbody->body->SetLinearVelocity(velocity);
        return;
    }

    HandleMovementInput();
    HandleTimers();
    HandleWallSlide();

    if (!disableAbilities) {
        jumpMechanic.Update(dt);
        dashMechanic.Update(dt);
    }
    attackMechanic.Update(dt);

    // Input por mando (LT)
    if (controller && SDL_GameControllerGetAttached(controller)) {
        static bool ltHeldPreviously = false;

        Sint16 triggerValue = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        bool triggerNow = triggerValue > 16000;

        bool hookDown = false;
        if (triggerNow && !ltHeldPreviously) {
            hookDown = true;
        }

        ltHeldPreviously = triggerNow;

        if (hookDown) {
            Engine::GetInstance().scene->GetHookManager()->TryUseClosestHook();
        }
    }

    if (isMoving && isLanding)
    {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastWalkSoundTime >= walkSoundInterval) {
            Engine::GetInstance().audio->PlayFx(soundWalkId, 1.0f, 0);
            lastWalkSoundTime = currentTime;
        }
    }

    // DEBUG: activar habilidades con teclas numéricas
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_1) == KEY_DOWN) {
        EnableJump(true);
        LOG("Jump habilitado");
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_2) == KEY_DOWN) {
        EnableDoubleJump(true);
        LOG("Double Jump habilitado");
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_3) == KEY_DOWN) {
        EnableDash(true);
        LOG("Dash habilitado");
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_4) == KEY_DOWN) {
        EnableGlide(true);
        LOG("Glide habilitado");
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_5) == KEY_DOWN) {
        EnableWallJump(true);
        LOG("Wall Jump habilitado");
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_6) == KEY_DOWN) {
        EnablePush(true);
        LOG("Push habilitado");
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_7) == KEY_DOWN) {
        SetHookUnlocked(true);
        LOG("Hook habilitado");
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_8) == KEY_DOWN) {
        //Engine::GetInstance().render.get()->ToggleCameraLock();
    }

    static bool prevAState = false;

    if (controller && SDL_GameControllerGetAttached(controller)) {
        aPressedNow = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
        aJustPressed = (aPressedNow && !prevAState);
        prevAState = aPressedNow;
    }

    bool spaceJustPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN;

    if (spaceJustPressed || aJustPressed) {
        player->GetMechanics()->GetMovementHandler()->pendingLandingCheck = false;
    }


    if (pendingLandingCheck) {
        if (player->IsTouchingPlatform()) {
            player->GetMechanics()->GetJumpMechanic()->OnLanding();
        }
    }

    UpdateAnimation();

    if (!hookUnlocked) return;

    // Input por teclado
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_L) == KEY_DOWN) {
        Engine::GetInstance().scene->GetHookManager()->TryUseClosestHook();
    }
}

void MovementHandler::HandleMovementInput() {
    if (cantMove) return;

    if (player->GetMechanics()->GetJumpMechanic()->IsWallJumpLocked()) {
        return; // bloquea movimiento lateral tras wall jump
    }

    b2Vec2 velocity = player->pbody->body->GetLinearVelocity();
    bool moved = false;

    if (!dashMechanic.IsDashing()) {
        // Comprobar entrada de gamepad
        if (controller && SDL_GameControllerGetAttached(controller)) {
            Sint16 axisX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
            const Sint16 deadZone = 8000;  // Zona muerta para evitar movimientos involuntarios

            if (axisX < -deadZone) {
                movementDirection = -1;
                velocity.x = movementDirection * speed;
                moved = true;
            }
            else if (axisX > deadZone) {
                movementDirection = 1;
                velocity.x = movementDirection * speed;
                moved = true;
            }
        }

        // Si no se mueve con el mando, intenta teclado
        if (!moved) {
            if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
                movementDirection = -1;
                velocity.x = movementDirection * speed;
                isMoving = true;
            }
            else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
                movementDirection = 1;
                velocity.x = movementDirection * speed;
                isMoving = true;
            }
            else {
                velocity.x = 0.0f;
                isMoving = false;
            }
        }
    }

    player->pbody->body->SetLinearVelocity(velocity);
}

void MovementHandler::UpdateAnimation() {

    if (player->GetMechanics()->GetHealthSystem()->IsInHitAnim()) return;
    if (player->GetState() == "die") return;
    if (dashMechanic.IsDashing()) return;

    std::string state = player->GetAnimation()->GetCurrentState();
    if (state == "walljump" || state == "transition" || state == "glide" || state == "doublejump" || state == "jump") {
        return;
    }

    if (!attackMechanic.IsAttacking() &&
        !dashMechanic.IsDashing() &&
        !jumpMechanic.IsJumping() &&
        !fallMechanic.IsFalling() &&
        !fallMechanic.IsStunned() &&
        !isWallSliding)
    {
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT ||
            Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
        {
            player->GetAnimation()->SetStateIfHigherPriority("run_right");
        }
        else if (player->GetAnimation()->GetCurrentState() == "run_right") {
            player->GetAnimation()->ForceSetState("idle");
        }
    }
}

void MovementHandler::EnableJump(bool enable) {
    jumpMechanic.Enable(enable);
}

void MovementHandler::EnableDoubleJump(bool enable) {
    jumpMechanic.EnableDoubleJump(enable);
}

void MovementHandler::EnableDash(bool enable) {
    dashMechanic.Enable(enable);
}

int MovementHandler::GetMovementDirection() const {
    return movementDirection;
}

bool MovementHandler::IsVisible() const {
    return visible;
}

void MovementHandler::OnWallCollision() {
    dashMechanic.OnWallCollision();
}

void MovementHandler::SetCantMove(bool cantMove) {
    this->cantMove = cantMove;
}

bool MovementHandler::CanAttack() const {
    return canAttack;
}

void MovementHandler::SetCanAttack(bool canAttack) {
    this->canAttack = canAttack;
}

void MovementHandler::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::DOOR:
        fallMechanic.OnLanding();
        jumpMechanic.OnLanding();

        Engine::GetInstance().scene->GetHookManager()->ResetUsedHooks();
        break;
    case ColliderType::PLATFORM:
    {
        lastPlatformCollider = physB;
        isLanding = true;
        fallMechanic.OnLanding();
        jumpMechanic.OnLanding();

        Engine::GetInstance().scene->GetHookManager()->ResetUsedHooks();

        break;
    }
    case ColliderType::BOX:
            jumpMechanic.OnLanding();
            fallMechanic.OnLanding();
            player->GetMechanics()->SetIsOnGround(true); 
        break;
    case ColliderType::WALL_SLIDE:
    {
        if (player->GetMechanics()->IsOnGround()) {
            break;
        }
        wallSlideFlip = movementDirection < 0;
        isWallSliding = true;
        player->GetMechanics()->SetIsWallSliding(true);
        isJumping = false;
        OnWallCollision();

        int dir = (movementDirection != 0) ? movementDirection : 1;
        player->GetMechanics()->GetWallSlideMechanic()->OnTouchWall(dir);
        jumpMechanic.jumpCount = 0;
    }
        break;

    case ColliderType::WALL:
    case ColliderType::DESTRUCTIBLE_WALL:
    {
        
        float playerX = player->GetPosition().getX();
        float wallX = physB->body->GetPosition().x * PIXELS_PER_METER;
    }
        break;
    case ColliderType::DOWN_CAMERA:
            LOG("DOWN_CAMERA collision detected, moviendo cámara abajo");
            Engine::GetInstance().render->downCameraActivated = true; // o el valor que necesites
        break;
    case ColliderType::LIANA:
            isOnLiana = true;
            lianaCenterX = METERS_TO_PIXELS(physB->body->GetPosition().x); // Guardar en PIXELES
            dashMechanic.CancelDash();
            player->pbody->body->SetLinearVelocity(b2Vec2_zero);
            disableAbilities = true; // Bloquear salto y dash
        break;
    case ColliderType::SPIKE:
        if (lastPlatformCollider != nullptr) {
            player->GetMechanics()->UpdateLastSafePosition(lastPlatformCollider);
        }
        break;
    default:
        break;
    }
}

void MovementHandler::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        isLanding = false;
        jumpCooldownTimer.Start();
        jumpCooldownActive = true;
        player->GetMechanics()->SetIsOnGround(false);
        break;
    case ColliderType::BOX:
        boxCooldownTimer.Start();
        boxCooldownActive = true;
        break;

    case ColliderType::WALL_SLIDE:
    {
        isWallSliding = false;
        player->GetMechanics()->SetIsWallSliding(false);
        wallSlideCooldownTimer.Start();
        wallSlideCooldownActive = true;
        player->pbody->body->SetGravityScale(2.0f); // Volver a gravedad normal
        player->GetAnimation()->SetStateIfHigherPriority("fall");

        break;
    }
    case ColliderType::WALL:
    case ColliderType::DESTRUCTIBLE_WALL:
        break;
    case ColliderType::DOWN_CAMERA:
        LOG("DOWN_CAMERA collision ended, reseteando offset cámara");
        Engine::GetInstance().render->downCameraActivated = false;
        downCameraCooldownTimer.Start();
        downCameraCooldownActive = true;
        break;
    case ColliderType::LIANA:
        printf("Saliendo de LIANA\n");
        isOnLiana = false;
        player->pbody->body->SetGravityScale(2.0f); // Restablece la gravedad normal
        disableAbilities = false; // Habilitar salto y dash otra vez
        lianaCooldownTimer.Start();
        lianaCooldownActive = true;
        if (player->IsTouchingPlatform()) {
            fallMechanic.OnLanding();
            jumpMechanic.OnLanding();
            player->SetState("idle"); // o lo que corresponda si se mueve
        }
        break;
    default:
        break;
    }
}


void MovementHandler::HandleTimers() {
    if (jumpCooldownActive && jumpCooldownTimer.ReadMSec() >= jumpCooldownTime) {
        jumpCooldownActive = false;
    }

    if (wallSlideCooldownActive && wallSlideCooldownTimer.ReadMSec() >= wallSlideCooldownTime) {
        wallSlideCooldownActive = false;
    }
    if (downCameraCooldownActive && downCameraCooldownTimer.ReadMSec() >= downCameraCooldownTime) {
        downCameraCooldownActive = false;
    }
    if (boxCooldownActive && boxCooldownTimer.ReadMSec() >= boxCooldownTime) {
        boxCooldownActive = false;
    }
    if (lianaCooldownActive && lianaCooldownTimer.ReadMSec() >= lianaCooldownTime) {
        lianaCooldownActive = false;
    }
}

void MovementHandler::HandleWallSlide() {
    if (wallSlideCooldownActive) {
        return;
    }
    if (isWallSliding) {
        b2Vec2 velocity = player->pbody->body->GetLinearVelocity();

        // Mantener velocidad X actual
        float currentX = velocity.x;

        // Forzar velocidad Y controlada (por ejemplo, deslizar m�s despacio)
        float controlledY = 2.0f; // velocidad vertical de descenso controlada

        player->pbody->body->SetLinearVelocity(b2Vec2(currentX, controlledY));

        player->SetState("wall_slide");
    }
}
MovementHandler::~MovementHandler() {
    if (controller) {
        SDL_GameControllerClose(controller);
        controller = nullptr;
    }
}

void MovementHandler::StartWallSlideCooldown() {
    wallSlideCooldownActive = true;
    wallSlideCooldownTimer.Start();
}

void MovementHandler::EnableGlide(bool enable) {
    jumpMechanic.EnableGlide(enable);
}

void MovementHandler::EnableWallJump(bool enable) {
    jumpMechanic.EnableWallJump(enable);
}
bool MovementHandler::IsWallJumpUnlocked() const {
    return jumpMechanic.IsWallJumpUnlocked();
}

void MovementHandler::EnablePush(bool enable) {
    canPushBoxes = enable;
}

bool MovementHandler::CanPush() const {
    return canPushBoxes;
}