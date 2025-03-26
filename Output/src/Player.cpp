#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "box2D/box2d.h"


Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {}

bool Player::Awake() {
	position = Vector2D(96, 96);
	return true;
}

bool Player::Start() {
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());

    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

    // Load animations with the texture (if necessary)
    animation.LoadAnimations(parameters, texture);

    // Create the body at the same position, and ensure it's centered
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), texW / 2, bodyType::DYNAMIC);
    pbody->listener = this;
    pbody->ctype = ColliderType::PLAYER;

    return true;
}

bool Player::Update(float dt) {
    
    if (isStunned) {
        pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        if (stunTimer.ReadSec() >= stunDuration) {
            isStunned = false;
            state = "idle";

            fallStartY = position.getY();
            fallEndY = position.getY();
            fallDistance = 0.0f;
        }
        return true;
    }
    
    HandleInput();
    HandleJump();
    HandleDash();
    HandleFall();
    HandleWallSlide();

    // Movimiento con f�sica

    if (!isDashing) {

        b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

        velocity.x = 0; // Reset horizontal por defecto

        if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
            velocity.x = -speed;
        }
        if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
            velocity.x = speed;
        }
        pbody->body->SetLinearVelocity(velocity);
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_1) == KEY_DOWN && !isJumping) {
        EnableJump(!jumpUnlocked);
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_2) == KEY_DOWN && !isJumping) {
        EnableDoubleJump(!doubleJumpUnlocked);
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_3) == KEY_DOWN && !isJumping) {
        EnableDash(!dashUnlocked);
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_4) == KEY_DOWN && !isJumping) {
        Engine::GetInstance().render.get()->StartCameraShake(5, 100);  
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_5) == KEY_DOWN && !isJumping) {
        Engine::GetInstance().render.get()->ToggleCameraLock();
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_6) == KEY_DOWN && !isJumping) {
        Engine::GetInstance().render.get()->ToggleVerticalOffsetLock();
    }

    // Apply the velocity
    b2Transform pbodyPos = pbody->body->GetTransform();

    // Update the position of the texture based on the body's position
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    // Update animation based on the new position
    animation.Update(dt, state, position.getX(), position.getY());

    return true;
}


void Player::HandleInput() {
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
        movementDirection = -1;
        state = "run_left";
	}
	else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
        movementDirection = 1;
		state = "run_right";
	}
	else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
		state = "attack";
	}
	else {
		state = "idle";
	}
}

bool Player::CleanUp() {
	LOG("Cleanup player");
	Engine::GetInstance().textures.get()->UnLoad(texture);
	return true;
}

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {
	case ColliderType::PLATFORM:
		//LOG("Collision PLATFORM");
        CheckFallImpact();
		isJumping = false;
        hasDoubleJumped = false;
        if (jumpUnlocked) {
            EnableJump(true);
        }
        isOnGround = true;
		break;
    case ColliderType::WALL:
        //LOG("Collision WALL");
        if (isDashing) {
            CancelDash();
        }

        isWallSliding = true;

        break;
	case ColliderType::ITEM:
		//LOG("Collision ITEM");
		Engine::GetInstance().physics.get()->DeletePhysBody(physB);
		break;
    case ColliderType::DOWN_CAMERA:
        if (!wasInDownCameraZone) {
            Engine::GetInstance().render.get()->ToggleVerticalOffsetLock();
            wasInDownCameraZone = true;
        }
	default:
		//LOG("Collision UNKNOWN");
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        isOnGround = false;
        break;
    case ColliderType::WALL:
        isWallSliding = false;
        break;
    case ColliderType::ITEM:
        break;
    case ColliderType::DOWN_CAMERA:
        wasInDownCameraZone = false;
        break;
    default:
        break;
    }
}

Vector2D Player::GetPosition() const {
	return position;
}

void Player::HandleJump() {

    if (!jumpUnlocked) return;
    // Obtener la velocidad actual del jugador
    b2Vec2 velocity = pbody->body->GetLinearVelocity();

    // --- INICIO DEL PRIMER SALTO ---
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping) {
        velocity.y = -jumpForce;  // Aplicamos la fuerza inicial del salto
        isJumping = true;
        state = "jump";
    }

    // --- DOBLE SALTO ---
    else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping && !hasDoubleJumped && doubleJumpUnlocked) {
        velocity.y = -jumpForce;  // Aplicamos la fuerza del doble salto
        hasDoubleJumped = true;   // Marcar que ya usamos el doble salto
    }

    // --- SALTO PROGRESIVO (Mientras se mantenga presionado) ---
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT && isJumping) {
        if (jumpTime < maxJumpTime) {
            velocity.y -= 0.3f;
            jumpTime += 0.016f;  // Aproximadamente 1 frame a 60 FPS
        }
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_UP && isJumping) {
        if (velocity.y < 0) {  // Si a�n est� subiendo, forzamos la ca�da
            velocity.y = 0;   // Reduce la velocidad de subida de golpe
        }
    }

    // --- GRAVEDAD SUAVE Y PROGRESIVA EN LA CA�DA ---
    if (velocity.y > 0 && !isDashing && !isWallSliding) {
        velocity.y += std::min(velocity.y * 0.1f, 0.5f);
    }


    // Aplicar la nueva velocidad al jugador
    pbody->body->SetLinearVelocity(velocity);
}

void Player::HandleDash() {

    if (!dashUnlocked) return;

    if (!canDash && dashCooldown.ReadSec() >= dashMaxCoolDown) {
        canDash = true;
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_K) == KEY_DOWN && canDash) {
        isDashing = true;
        canDash = false;
        dashCooldown.Start();

        dashStartPosition = position;

        pbody->body->SetGravityScale(0.0f);

        Engine::GetInstance().render.get()->DashCameraImpulse(movementDirection, 100); 
    }

    if (isDashing) {
        // Aplica velocidad
        b2Vec2 vel(dashSpeed * movementDirection, 0.0f);
        pbody->body->SetLinearVelocity(vel);

        // Calcula distancia recorrida
        float distance = abs(position.getX() - dashStartPosition.getX());

        if (distance >= maxDashDistance) {
            if (distance >= maxDashDistance) {
                CancelDash();
            }
        }
    }
}

void Player::HandleFall() {
    b2Vec2 velocity = pbody->body->GetLinearVelocity();

    if (velocity.y > 0.1f && !isJumping) {
        isJumping = true;
        fallStartY = position.getY(); 
        state = "fall";
    }
}

void Player::CheckFallImpact() {
    fallEndY = position.getY();
    fallDistance = fallEndY - fallStartY;

    if (fallDistance >= fallDistanceThreshold) {
        isStunned = true;
        state = "stunned";
        stunTimer.Start();
        Engine::GetInstance().render.get()->StartCameraShake(1, 1);
    }
}

void Player::HandleWallSlide()
{
    if (isWallSliding)
    {
        pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        state = "wall_slide";
    }
}

void Player::CancelDash() {
    isDashing = false;
    pbody->body->SetGravityScale(1.0f);

    b2Vec2 stop = pbody->body->GetLinearVelocity();
    stop.x = 0.0f;
    pbody->body->SetLinearVelocity(stop);
}
