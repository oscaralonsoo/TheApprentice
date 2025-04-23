// PlayerMechanics.cpp
#include "PlayerMechanics.h"
#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Physics.h"
#include "Log.h"
#include "Audio.h"

void PlayerMechanics::Init(Player* player) {
    this->player = player;
    slimeFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/slime_move.ogg", 0.1f);
    jumpFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/slime_jump.ogg", 1.0f);

}

void PlayerMechanics::Update(float dt) {

    if( Engine::GetInstance().scene->saving == true)
        return;
    if (godMode) {
        b2Vec2 velocity = player->pbody->body->GetLinearVelocity();
        velocity.x = 0.0f;
        velocity.y = 0.0f;

        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT) {
            velocity.y = -8.0f;
        }
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
            velocity.y = 8.0f;
        }
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
            velocity.x = -8.0f;
        }
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
            velocity.x = 8.0f;
        }

        player->pbody->body->SetLinearVelocity(velocity);
        player->SetState("idle");  // O puedes poner una animación de vuelo si la tienes
        return;
    }

    if (wallSlideCooldownActive && wallSlideCooldownTimer.ReadMSec() >= wallSlideCooldownTime) {
        wallSlideCooldownActive = false;
    }

    if (wallCooldownActive && wallCooldownTimer.ReadMSec() >= wallCooldownTime) {
        wallCooldownActive = false;
    }

    if (jumpCooldownActive && jumpCooldownTimer.ReadMSec() >= jumpCooldownTime) {
        jumpCooldownActive = false;
    }

    if (!inDownCameraZone && downCameraCooldown.ReadMSec() >= downCameraCooldownTime) {
        inDownCameraZone = false;
    }

    if (shouldRespawn && !godMode) {
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
        player->pbody->body->SetLinearVelocity(b2Vec2_zero);
        Engine::GetInstance().scene->SaveGameXML();
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

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN && attackSensor == nullptr && canAttack && !isDashing) {
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
        int offsetX = (movementDirection > 0) ? 38 : -38;
        int playerX = METERS_TO_PIXELS(player->pbody->body->GetPosition().x) + offsetX;
        int playerY = METERS_TO_PIXELS(player->pbody->body->GetPosition().y) - 10;

        b2Vec2 newPos(PIXEL_TO_METERS(playerX), PIXEL_TO_METERS(playerY));
        attackSensor->body->SetTransform(newPos, 0);

        if (attackTimer.ReadMSec() >= attackDuration) {
            DestroyAttackSensor();
        }
    }

    HandleSound();
}

void PlayerMechanics::PostUpdate()
{
    HandleLifes();
}

void PlayerMechanics::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        if (!jumpCooldownActive)    
        {
            CheckFallImpact();
            isJumping = false;  
            isOnGround = true;
            if (jumpUnlocked) EnableJump(true);
            jumpCount = 0;
            if (isFalling) {
                isFalling = false;
                CheckFallImpact();
                player->pbody->body->SetLinearVelocity(b2Vec2_zero);
            }
        }
        break;
    case ColliderType::WALL_SLIDE:
        if (!wallSlideCooldownActive && !isOnGround) {
            isWallSliding = true;
            isJumping = false;
        }
        break;
    case ColliderType::WALL:
    case ColliderType::DESTRUCTIBLE_WALL:
        if (!wallCooldownActive) {
            isTouchingWall = true;
            isJumping = false;
            if (isDashing) {
                CancelDash(); 
            }
        }
        break;
    case ColliderType::DOWN_CAMERA:
        if (!inDownCameraZone) {
            inDownCameraZone = true;
            downCameraCooldown.Start();
            originalCameraOffsetY = Engine::GetInstance().render->cameraOffsetY;
            Engine::GetInstance().render->cameraOffsetY = 300;
        }
        break;
    case ColliderType::SAVEGAME:
        Engine::GetInstance().scene->saveGameZone = true;
        break;
    case ColliderType::ENEMY:
        if (!isInvulnerable && !godMode && physA->ctype == ColliderType::PLAYER_DAMAGE) {
            vidas -= 1;
            StartInvulnerability();
            Engine::GetInstance().render->StartCameraShake(0.5, 1);
        }
        break;
    case ColliderType::SPIKE:
        if (!godMode) {
            player->SetState("dead");
            UpdateLastSafePosition();
            shouldRespawn = true;
        }
        break;
    case ColliderType::PUSHABLE_PLATFORM:
        // Se comporta como una plataforma: permite saltar y aterrizar
        if (!jumpCooldownActive) {
            isJumping = false;
            isOnGround = true;
            if (jumpUnlocked) EnableJump(true);
            jumpCount = 0;

            // También se comporta como una pared: cancela el dash si está activo
            if (isDashing) {
                CancelDash();
            }
        }
        break;
    default:
        break;
    }
}

void PlayerMechanics::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        printf("Sale a la plataforma");
        isOnGround = false;
        lasMovementDirection = movementDirection;
        lastPlatformCollider = physB;
        break;
    case ColliderType::WALL_SLIDE:
        isWallSliding = false;
        wallSlideCooldownTimer.Start();
        wallSlideCooldownActive = true;
        player->pbody->body->SetGravityScale(2.0f);
        break;
    case ColliderType::WALL:
    case ColliderType::DESTRUCTIBLE_WALL:
        isTouchingWall = false;
        wallCooldownTimer.Start();
        wallCooldownActive = true;
        break;
    case ColliderType::DOWN_CAMERA:
        if (inDownCameraZone && downCameraCooldown.ReadSec() >= downCameraCooldownTime) {
            inDownCameraZone = false;
            downCameraCooldown.Start();
            Engine::GetInstance().render->cameraOffsetY = originalCameraOffsetY;
        }
        break;
    case ColliderType::PUSHABLE_PLATFORM:
        isOnGround = false;
        lasMovementDirection = movementDirection;
        lastPlatformCollider = physB;
        jumpCooldownTimer.Start();
        jumpCooldownActive = true;
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

    // Inicio del salto o doble salto
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
        if (isOnGround || (doubleJumpUnlocked && jumpCount < maxJumpCount)) {
            velocity.y = -jumpForce; // sin el 0.8f, para dar un impulso inicial más fuerte
            isJumping = true;
            isHoldingJump = true;
            jumpStartY = player->GetPosition().getY(); // guardamos altura de inicio
            jumpCount++;
            isOnGround = false;
            player->SetState("jump");

            jumpCooldownTimer.Start();
            jumpCooldownActive = true;

            playJumpSound = true;
        }
    }

    if (isHoldingJump && Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT) {
        float currentY = player->GetPosition().getY();
        float heightJumped = jumpStartY - currentY;

        if (heightJumped >= minHoldJumpHeight && heightJumped < maxJumpHeight) {
            float t = (heightJumped - minHoldJumpHeight) / (maxJumpHeight - minHoldJumpHeight);
            float forceFactor = jumpHoldForceFactor * exp(-jumpDecayRate * t); // estilo Hollow Knight

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

    // Caída rápida automática al soltar salto (estilo Hollow Knight)
    if (isJumping && !isHoldingJump && !isWallSliding) {
        velocity.y += fallAccelerationFactor; // acelera el descenso
    }

    player->pbody->body->SetLinearVelocity(velocity);
}

void PlayerMechanics::HandleDash() {

    if (!dashUnlocked) return;

    if (isTouchingWall) return;

    if (!canDash && dashCooldown.ReadSec() >= dashMaxCoolDown) {
        canDash = true;
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_K) == KEY_DOWN && canDash) {
        isDashing = true;
        canDash = false;
        dashCooldown.Start();

        dashStartPosition = player->GetPosition();

        player->pbody->body->SetGravityScale(0.0f);

        // Dirección fija del dash
        dashDirection = movementDirection;

        if (isWallSliding) {
            dashDirection *= -1;
        }

        Engine::GetInstance().render->DashCameraImpulse(dashDirection, 100);

        if (attackSensor != nullptr) DestroyAttackSensor();
    }

    if (isDashing) {
        b2Vec2 vel(dashSpeed * dashDirection, 0.0f);
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

    if (velocity.y > 0.1f && !isWallSliding) {
        if (!isFalling) {
            isFalling = true;
            fallStartY = player->GetPosition().getY();
            player->SetState("fall");
            isJumping = true;
            isOnGround = false;
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
        player->SetState("landing_stun");
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
    int offsetX = (movementDirection > 0) ? 38 : -38;

    playerAttackX = METERS_TO_PIXELS(player->pbody->body->GetPosition().x) + offsetX;
    playerAttackY = METERS_TO_PIXELS(player->pbody->body->GetPosition().y);

    attackSensor = Engine::GetInstance().physics->CreateRectangleSensor(
        playerAttackX, playerAttackY - 10,
        32, 64,
        KINEMATIC,
        CATEGORY_ATTACK,     
        CATEGORY_ENEMY       
    );
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
        player->SetState("idle");
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

void PlayerMechanics::HandleSound()
{
    bool isGroundedAndMoving =
        isOnGround &&
        !isJumping &&
        !isFalling &&
        !isDashing &&
        !isStunned &&
        (
            Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT ||
            Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT
            );

    if (isGroundedAndMoving) {
        if (!isSlimeSoundPlaying && slimeFxId > 0) {
            slimeChannel = Engine::GetInstance().audio->PlayFxReturnChannel(slimeFxId, 0.3f, -1);
            if (slimeChannel != -1) {
                isSlimeSoundPlaying = true;
            }
        }
    }
    else {
        if (isSlimeSoundPlaying && slimeChannel != -1) {
            Mix_HaltChannel(slimeChannel);
            isSlimeSoundPlaying = false;
            slimeChannel = -1;
        }
    }

    // Sonido de salto (solo una vez cuando salta)
    if (playJumpSound && jumpFxId > 0) {
        Engine::GetInstance().audio->PlayFx(jumpFxId, 0.3f, 0);
        playJumpSound = false;
    }
}

void PlayerMechanics::HandleLifes()
{
    if (vidas <= 0) 
    {
        printf("ENTRAAAAAAA");
        Engine::GetInstance().scene.get()->isDead = true;
        vidas = 3;
    }
}