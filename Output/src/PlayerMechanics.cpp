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
    if( Engine::GetInstance().scene->saving == true)
        return;

    if (wallSlideCooldownActive && wallSlideCooldownTimer.ReadSec() >= wallSlideCooldownTime) {
        wallSlideCooldownActive = false;
    }

    if (shouldRespawn) {
        shouldRespawn = false;
        player->SetPosition(lastPosition);
        player->pbody->body->SetLinearVelocity(b2Vec2_zero);

        player->SetState("idle");

        Engine::GetInstance().render->StartCameraShake(0.5, 2);

        StartInvulnerability();

        isStunned = true;
        stunTimer.Start();
        return;
    }


    // Invulnerabilidad temporal
    if (isInvulnerable) {
        if (invulnerabilityTimer.ReadSec() >= invulnerabilityDuration) {
            isInvulnerable = false;
            visible = true;
        }
        else {
            if (blinkTimer.ReadMSec() >= blinkInterval) {
                visible = !visible; // toggle de visibilidad
                blinkTimer.Start();
                blinkInterval += 150.0f; // para que parpadee más lento con el tiempo
            }
        }
    }


    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_DOWN && Engine::GetInstance().scene->saveGameZone) {
        // TODO JAVI ---- Si guardas mientras te mueves en el eje x, te guardas moviendote y tendrias que estar quieto
        player->pbody->body->SetLinearVelocity(b2Vec2_zero);
        Engine::GetInstance().scene->SaveGameXML();
        return;
    }

    if (isStunned) {
        player->pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        if (stunTimer.ReadSec() >= stunDuration) {
            isStunned = false;
            player->SetState("landing_stun");

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

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN && attackSensor == nullptr && canAttack) {
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
        if (isFalling) {
            isFalling = false;
            CheckFallImpact();
            // Frena completamente al tocar el suelo tras una caída
            player->pbody->body->SetLinearVelocity(b2Vec2_zero);
        }

        break;
    case ColliderType::WALL_SLIDE:
        if (!wallSlideCooldownActive) {
            printf(">> COLISIËN WALL SLIDE\n");
            isWallSliding = true;
            isJumping = false;
        }
        break;
    case ColliderType::WALL:
        if (isDashing) CancelDash();
        isJumping = false;
        break;
    case ColliderType::ITEM:
        Engine::GetInstance().physics->DeletePhysBody(physB);
        break;
    case ColliderType::DOWN_CAMERA:
        Engine::GetInstance().render->SetDownCameraActive(true);
        break;
        break;
    case ColliderType::SAVEGAME:
        Engine::GetInstance().scene->saveGameZone = true;
        break;
    case ColliderType::ENEMY:
        if (!isInvulnerable)
        {

        }
        break;
    case ColliderType::SPIKE:
        UpdateLastSafePosition();
        shouldRespawn = true;
        break;
    default:
        break;
    }
}

void PlayerMechanics::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        isOnGround = false;
        lasMovementDirection = movementDirection;
        lastPlatformCollider = physB;
        break;
    case ColliderType::WALL_SLIDE:
        printf(">>> SALIDA WALL SLIDE\n");
        isWallSliding = false;
        wallSlideCooldownTimer.Start();
        wallSlideCooldownActive = true;
        player->pbody->body->SetGravityScale(2.0f);
        break;
    case ColliderType::WALL: break;
    case ColliderType::DOWN_CAMERA:
        Engine::GetInstance().render->SetDownCameraActive(false);
        break;
    case ColliderType::SAVEGAME: Engine::GetInstance().scene->saveGameZone = false; break;
    default: break;
    }
}

void PlayerMechanics::HandleInput() {

    // Si no se está pulsando izquierda ni derecha, y no está saltando, cayendo, atacando o en wallslide...
    bool noMovimiento = Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) != KEY_REPEAT &&
        Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) != KEY_REPEAT;

    if (noMovimiento && !isJumping && !isFalling && !isAttacking && !isWallSliding && !isDashing && !isStunned) {
        player->SetState("idle");
    }

    if (!isAttacking) {
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
            movementDirection = -1;
            if (!isFalling && !isJumping && !isWallSliding) {
                player->SetState("run_right");
            }
        }
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_1) == KEY_REPEAT) {
            player->SetState("idle");
        }
        else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
            movementDirection = 1;
            if (!isFalling && !isJumping && !isWallSliding) {
                player->SetState("run_right");
            }
        }
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
        player->SetState("attack");
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
    player->pbody->body->SetGravityScale(2.0f);
    b2Vec2 stop = player->pbody->body->GetLinearVelocity();
    stop.x = 0.0f;
    player->pbody->body->SetLinearVelocity(stop);
}

void PlayerMechanics::HandleFall() {
    b2Vec2 velocity = player->pbody->body->GetLinearVelocity();

    if (velocity.y > 0.5f && !isWallSliding) {
        if (!isFalling) {
            isFalling = true;
            fallStartY = player->GetPosition().getY();
            player->SetState("fall");
        }
    }
    else if (isOnGround || isWallSliding) {
        if (isFalling) {
            isFalling = false;
            CheckFallImpact();
        }
    }
}

void PlayerMechanics::CheckFallImpact() {
    fallEndY = player->GetPosition().getY();
    fallDistance = fallEndY - fallStartY;

    if (fallDistance >= fallDistanceThreshold) {
        isStunned = true;
        player->SetState("landing_stun"); // Activamos la animación de aterrizaje forzoso
        stunTimer.Start();
        Engine::GetInstance().render->StartCameraShake(1, 1);
    }
}

void PlayerMechanics::HandleWallSlide() {
    if (isWallSliding) {
        printf(">>> Wall slide activo\n");
        player->pbody->body->SetGravityScale(8.0f);
        player->pbody->body->SetLinearVelocity(b2Vec2_zero);
        player->SetState("wall_slide");
    }
}

void PlayerMechanics::CreateAttackSensor() {
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

void PlayerMechanics::StartInvulnerability() {
    isInvulnerable = true;
    invulnerabilityTimer.Start();
    blinkTimer.Start();
    visible = true;
    blinkInterval = 150.0f;
}

void PlayerMechanics::UpdateLastSafePosition() {
    if (!lastPlatformCollider || !lastPlatformCollider->body) return;

    float width = lastPlatformCollider->width;
    float height = lastPlatformCollider->height;
    b2Vec2 posMeters = lastPlatformCollider->body->GetPosition();

    float topY = METERS_TO_PIXELS(posMeters.y) - (height / 2.0f);
    float verticalOffset = 100.0f;
    float respawnY = topY - verticalOffset;

    float platformCenterX = METERS_TO_PIXELS(posMeters.x);
    float platformHalfWidth = width / 2.0f;

    float respawnX = 0.0f;

    if (lasMovementDirection > 0) {
        respawnX = platformCenterX + platformHalfWidth - 20;
    }
    else {
        respawnX = platformCenterX - platformHalfWidth - 103;
    }

    lastPosition = Vector2D(respawnX, respawnY);
}