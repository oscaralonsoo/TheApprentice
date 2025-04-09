// PlayerMechanics.cpp
#include "PlayerMechanics.h"
#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Physics.h"
#include "Log.h"

void PlayerMechanics::Init(Player* player) {
    this->player = player;
}

void PlayerMechanics::Update(float dt) {
    if (shouldRespawn) {
        shouldRespawn = false;
        player->SetPosition(respawnPosition);

        // Reset de velocidad por si acaso
        player->pbody->body->SetLinearVelocity(b2Vec2_zero);

        // También puedes resetear estados si hace falta
        isJumping = false;
        hasDoubleJumped = false;
        isDashing = false;
        isWallSliding = false;

        player->SetState("idle");

        Engine::GetInstance().render->StartCameraShake(0.5, 2);

        StartInvulnerability(); 

        isStunned = true;
        stunTimer.Start();
        return; // Opcional, si quieres que no haga más cosas ese frame
    }

    // Invulnerabilidad temporal
    if (isInvulnerable) {
        if (invulnerabilityTimer.ReadSec() >= invulnerabilityDuration) {
            isInvulnerable = false;
            visible = true; // se queda visible al final
        }
        else {
            if (blinkTimer.ReadMSec() >= blinkInterval) {
                visible = !visible; // toggle de visibilidad
                printf("VISIBLE: %s\n", visible ? "sí" : "no");
                blinkTimer.Start();
                blinkInterval += 150.0f;
            }
        }
    }
    
    if( Engine::GetInstance().scene->saving == true)
        return;
    if (cantMove)
    {
        return;
    }
    if (isStunned) {
        player->pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        if (stunTimer.ReadSec() >= stunDuration) {
            isStunned = false;
            player->SetState("idle");

            fallStartY = player->GetPosition().getY();
            fallEndY = fallStartY;
            fallDistance = 0.0f;
        }
        return;
    }

    HandleInput();
    HandleWallSlide();
    HandleJump();
    HandleDash();
    HandleFall();

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN && attackSensor == nullptr) {
        CreateAttackSensor();
    }

    if (!isDashing) {
        b2Vec2 velocity(0, player->pbody->body->GetLinearVelocity().y);

        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
            velocity.x = -speed;
        }
        else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
            velocity.x = speed;
        }

        player->pbody->body->SetLinearVelocity(velocity);
    }

    if (attackSensor != nullptr) {
        int offsetX = (movementDirection > 0) ? 60 : -15;
        int playerX = METERS_TO_PIXELS(player->pbody->body->GetPosition().x) + offsetX;
        int playerY = METERS_TO_PIXELS(player->pbody->body->GetPosition().y);

        b2Vec2 newPos(PIXEL_TO_METERS(playerX), PIXEL_TO_METERS(playerY));
        attackSensor->body->SetTransform(newPos, 0);

        if (attackTimer.ReadMSec() >= attackDuration) {
            DestroyAttackSensor();
        }
    }
}

void PlayerMechanics::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        CheckFallImpact();
        isJumping = false;
        hasDoubleJumped = false;
        if (jumpUnlocked) EnableJump(true);
        isOnGround = true;
        lastPlatformCollider = physB;
        break;
    case ColliderType::WALL_SLIDE:
        if (isDashing) CancelDash();
        isWallSliding = true;
        isJumping = false;
        break;
    case ColliderType::WALL:
        if (isDashing) CancelDash();
        isJumping = false;
        break;
    case ColliderType::ITEM:
        Engine::GetInstance().physics->DeletePhysBody(physB);
        break;
    case ColliderType::DOWN_CAMERA:
        if (!wasInDownCameraZone) {
            Engine::GetInstance().render->ToggleVerticalOffsetLock();
            wasInDownCameraZone = true;
        }
        break;
    case ColliderType::SPIKE:
        if (lastPlatformCollider) {
            UpdateLastSafePosition(lastPlatformCollider);
            respawnPosition = lastPosition;
        }
        else {
            // Si no hay plataforma previa, reaparece donde estaba (por seguridad)
            respawnPosition = player->GetPosition();
        }

        shouldRespawn = true;
        break;
    case ColliderType::SAVEGAME:
        Engine::GetInstance().scene->saveGameZone = true;
        break;
    case ColliderType::ENEMY:
        if (!isInvulnerable)    
        {

        }
        break;
    default:
        break;
    }
}

void PlayerMechanics::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        isOnGround = false;
        if (lastPlatformCollider) {
            UpdateLastSafePosition(lastPlatformCollider); 
        }
        break;
    case ColliderType::WALL_SLIDE: 
        isWallSliding = false;
        player->pbody->body->SetGravityScale(1.0f);
        break;
    case ColliderType::WALL: break;
    case ColliderType::DOWN_CAMERA: wasInDownCameraZone = false; break;
    case ColliderType::SAVEGAME: Engine::GetInstance().scene->saveGameZone = false; break;
    default: break;
    }
}

void PlayerMechanics::HandleInput() {
    if (!isAttacking) {
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
            movementDirection = -1;
            player->SetState("run_left");
        }
        else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
            movementDirection = 1;
            player->SetState("run_right");
        }
        else {
            player->SetState("idle");
        }
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
        player->SetState("attack");
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_DOWN && Engine::GetInstance().scene->saveGameZone) {
        // TODO JAVI ---- Si guardas mientras te mueves en el eje x, te guardas moviendote y tendrias que estar quieto
        b2Vec2 currentVelocity = player->pbody->body->GetLinearVelocity();
        currentVelocity.x = 0; 
        player->pbody->body->SetLinearVelocity(currentVelocity);
        Engine::GetInstance().scene->SaveGameXML();
    }
}


void PlayerMechanics::HandleJump() {
    if (!jumpUnlocked) return;

    b2Vec2 velocity = player->pbody->body->GetLinearVelocity();

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping && !isWallSliding) {
        velocity.y = -jumpForce;
        isJumping = true;
        player->SetState("jump");
    }
    else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping && !hasDoubleJumped && doubleJumpUnlocked && !isWallSliding) {
        velocity.y = -jumpForce;
        hasDoubleJumped = true;
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT && isJumping && !isWallSliding) {
        if (jumpTime < maxJumpTime) {
            velocity.y -= 0.3f;
            jumpTime += 0.016f;
        }
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_UP && isJumping) {
        if (velocity.y < 0) {
            velocity.y = 0;
        }
    }

    if (velocity.y > 0 && !isDashing && !isWallSliding) {
        velocity.y += std::min(velocity.y * 0.1f, 0.5f);
    }

    player->pbody->body->SetLinearVelocity(velocity);
}

void PlayerMechanics::HandleDash() {
    if (!dashUnlocked || isWallSliding) return;

    if (!canDash && dashCooldown.ReadSec() >= dashMaxCoolDown) {
        canDash = true;
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_K) == KEY_DOWN && canDash) {
        isDashing = true;
        canDash = false;
        dashCooldown.Start();

        dashStartPosition = player->GetPosition();

        player->pbody->body->SetGravityScale(0.0f);
        Engine::GetInstance().render->DashCameraImpulse(movementDirection, 100);

        if (attackSensor != nullptr) DestroyAttackSensor();
    }

    if (isDashing) {
        b2Vec2 vel(dashSpeed * movementDirection, 0.0f);
        player->pbody->body->SetLinearVelocity(vel);

        float distance = abs(player->GetPosition().getX() - dashStartPosition.getX());
        if (distance >= maxDashDistance) CancelDash();
    }
}

void PlayerMechanics::CancelDash() {
    isDashing = false;
    player->pbody->body->SetGravityScale(1.0f);
    b2Vec2 stop = player->pbody->body->GetLinearVelocity();
    stop.x = 0.0f;
    player->pbody->body->SetLinearVelocity(stop);
}

void PlayerMechanics::HandleFall() {
    b2Vec2 velocity = player->pbody->body->GetLinearVelocity();

    if (velocity.y > 0.5f && !isJumping && !isWallSliding) {
        isJumping = true;
        fallStartY = player->GetPosition().getY();
        player->SetState("fall");
    }
}

void PlayerMechanics::CheckFallImpact() {
    fallEndY = player->GetPosition().getY();
    fallDistance = fallEndY - fallStartY;

    if (fallDistance >= fallDistanceThreshold) {
        isStunned = true;
        player->SetState("stunned");
        stunTimer.Start();
        Engine::GetInstance().render->StartCameraShake(1, 1);
    }
}

void PlayerMechanics::HandleWallSlide() {
    if (isWallSliding) {
        player->pbody->body->SetGravityScale(5.0f);
        player->pbody->body->SetLinearVelocity(b2Vec2_zero);
        player->SetState("wall_slide");
    }
}

void PlayerMechanics::CreateAttackSensor() {
    if (!canAttack) return; 
    int offsetX = (movementDirection > 0) ? 60 : -15;

    playerAttackX = METERS_TO_PIXELS(player->pbody->body->GetPosition().x) + offsetX;
    playerAttackY = METERS_TO_PIXELS(player->pbody->body->GetPosition().y);

    attackSensor = Engine::GetInstance().physics->CreateRectangleSensor(playerAttackX, playerAttackY, 32, 64, KINEMATIC);
    attackSensor->ctype = ColliderType::ATTACK;
    attackSensor->listener = player;

    isAttacking = true;
    attackTimer.Start();
}

void PlayerMechanics::DestroyAttackSensor() {
    if (attackSensor != nullptr) {
        Engine::GetInstance().physics->DeletePhysBody(attackSensor);
        attackSensor = nullptr;
        isAttacking = false;
    }
}

void PlayerMechanics::UpdateLastSafePosition(PhysBody* platformBody) {
    if (!platformBody || !platformBody->body) return;

    float width = platformBody->width;
    float height = platformBody->height;

    b2Vec2 posMeters = platformBody->body->GetPosition();

    float topY = METERS_TO_PIXELS(posMeters.y) - (height / 2.0f);

    float verticalOffset = 100.0f;

    float horizontalOffset = METERS_TO_PIXELS(posMeters.x) + METERS_TO_PIXELS(posMeters.x) / 2;


    float respawnY = topY - verticalOffset;

    if (movementDirection > 0) {
        float respawnX = horizontalOffset + horizontalOffset;
        lastPosition = Vector2D(respawnX, respawnY);
    }
    else {
        float respawnX = horizontalOffset - horizontalOffset;
        lastPosition = Vector2D(respawnX, respawnY);
    }

    printf("POSICIÓN SEGURA ACTUALIZADA: X = %.2f, Y = %.2f (Dir: %d, Plataforma ancho: %.2f)\n", lastPosition.getX(), lastPosition.getY(), movementDirection, width);
}

void PlayerMechanics::StartInvulnerability() {
    isInvulnerable = true;
    invulnerabilityTimer.Start();
    blinkTimer.Start(); // esto es clave
    visible = true;
    blinkInterval = 150.0f;
}