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

    // Inicializar las mecánicas después de preparar el controller
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
}

void MovementHandler::Update(float dt) {
    fallMechanic.Update(dt);
    if (fallMechanic.IsStunned())
    {
        return;
    }
    HandleMovementInput();
    HandleTimers();
    HandleWallSlide();

    jumpMechanic.Update(dt);
    dashMechanic.Update(dt);
    attackMechanic.Update(dt);


    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_L) == KEY_DOWN)
    {
        Engine::GetInstance().scene->GetHookManager()->TryUseClosestHook();
    }

    UpdateAnimation();
}

void MovementHandler::HandleMovementInput() {
    if (cantMove) return;

    b2Vec2 velocity = player->pbody->body->GetLinearVelocity();
    bool moved = false;

    if (!attackMechanic.IsAttacking() && !dashMechanic.IsDashing()) {
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
            }
            else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
                movementDirection = 1;
                velocity.x = movementDirection * speed;
            }
            else {
                velocity.x = 0.0f;
            }
        }
    }

    player->pbody->body->SetLinearVelocity(velocity);
}

void MovementHandler::UpdateAnimation() {

    if (player->GetMechanics()->GetHealthSystem()->IsInHitAnim()) return;
    if (player->GetState() == "die") return;
    if (dashMechanic.IsDashing()) return;

    if (!attackMechanic.IsAttacking() &&
        !dashMechanic.IsDashing() &&
        !jumpMechanic.IsJumping() &&
        !fallMechanic.IsFalling() &&
        !fallMechanic.IsStunned() &&
        !isWallSliding)
    {
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT ||
            Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
            player->SetState("run_right");
        }
        else {
            player->SetState("idle");
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
    case ColliderType::PLATFORM:
    case ColliderType::PUSHABLE_PLATFORM:
    {
        if (!jumpCooldownActive) {
            jumpMechanic.OnLanding();
            fallMechanic.OnLanding();

            HookAnchor* hook = Engine::GetInstance().scene->GetActiveHook();
            if (hook) {
                hook->ResetHook();
                Engine::GetInstance().scene->GetHookManager()->RegisterHook(hook);
                LOG("Gancho reactivado tras aterrizar");
            }
        }
        break;
    }
    case ColliderType::WALL_SLIDE: // <<< Aquí nuevo
        if (!wallSlideCooldownActive) {
            isWallSliding = true;      // <<< ACTIVAR AQUÍ
            isJumping = false;
            if (player->GetMechanics()->IsOnGround()) {
                player->SetState("idle");
            }
        }
        break;

    case ColliderType::WALL:
    case ColliderType::DESTRUCTIBLE_WALL:
        if (!wallSlideCooldownActive) {
            float playerX = player->GetPosition().getX();
            float wallX = physB->body->GetPosition().x * PIXELS_PER_METER; 

            int dir = (playerX < wallX) ? -1 : 1; 

            wallSlideMechanic.OnTouchWall(dir);
        }
        break;
    default:
        break;
    }
}

void MovementHandler::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
    case ColliderType::PUSHABLE_PLATFORM:
        jumpCooldownTimer.Start();
        jumpCooldownActive = true;
        player->GetMechanics()->SetIsOnGround(false);
        break;

    case ColliderType::WALL_SLIDE: // <<< Aquí nuevo
        isWallSliding = false;     // <<< DESACTIVAR AQUÍ
        wallSlideCooldownTimer.Start();
        wallSlideCooldownActive = true;
        player->pbody->body->SetGravityScale(2.0f); // Volver a gravedad normal
        break;

    case ColliderType::WALL:
    case ColliderType::DESTRUCTIBLE_WALL:
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
}

void MovementHandler::HandleWallSlide() {
    if (isWallSliding) {
        b2Vec2 velocity = player->pbody->body->GetLinearVelocity();

        // Mantener velocidad X actual
        float currentX = velocity.x;

        // Forzar velocidad Y controlada (por ejemplo, deslizar más despacio)
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