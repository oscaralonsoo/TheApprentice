#include "MovementHandler.h"
#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Physics.h"

void MovementHandler::Init(Player* player) {
    this->player = player;

    jumpMechanic.Init(player);
    dashMechanic.Init(player);
    attackMechanic.Init(player);
    fallMechanic.Init(player);
    wallSlideMechanic.Init(player);
}

void MovementHandler::Update(float dt) {
    HandleMovementInput();
    HandleTimers();
    HandleWallSlide(); // <<< AÑADIR ESTA LLAMADA

    jumpMechanic.Update(dt);
    dashMechanic.Update(dt);
    attackMechanic.Update(dt);
    fallMechanic.Update(dt);

    UpdateAnimation();
}

void MovementHandler::HandleMovementInput() {
    if (cantMove)
        return;

    b2Vec2 velocity = player->pbody->body->GetLinearVelocity(); // coger el velocity actual completo

    if (!attackMechanic.IsAttacking() && !dashMechanic.IsDashing()) {
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


    player->pbody->body->SetLinearVelocity(velocity); // aquí aplicamos SOLO el x
}

void MovementHandler::UpdateAnimation() {
    if (!attackMechanic.IsAttacking() &&
        !dashMechanic.IsDashing() &&
        !jumpMechanic.IsJumping() &&
        !fallMechanic.IsFalling() &&
        !isWallSliding)
    {
        // Ahora sí puedo cambiar a idle o run_right
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
        if (!jumpCooldownActive) {
            jumpMechanic.OnLanding();
            fallMechanic.OnLanding();
        }
        break;

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
            wallSlideMechanic.OnTouchWall();
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