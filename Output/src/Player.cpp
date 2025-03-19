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
    HandleInput();
    HandleJump();
    HandleDash();

    // Movimiento con física
    b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
        velocity.x = -speed;
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
        velocity.x = speed;
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping) {
        pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
        isJumping = true;
    }

    // Apply the velocity
    pbody->body->SetLinearVelocity(velocity);
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
	else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
		state = "jump";
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
		LOG("Collision PLATFORM");
		isJumping = false;
        hasDoubleJumped = false;
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		Engine::GetInstance().physics.get()->DeletePhysBody(physB);
		break;
	default:
		LOG("Collision UNKNOWN");
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
	LOG("End Collision");
}

Vector2D Player::GetPosition() const {
	return position;
}

void Player::HandleJump() {
    // Obtener la velocidad actual del jugador
    b2Vec2 velocity = pbody->body->GetLinearVelocity();

    if (!canJump) return;


    // --- INICIO DEL PRIMER SALTO ---
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping) {
        velocity.y = -jumpForce;  // Aplicamos la fuerza inicial del salto
        isJumping = true;
    }

    // --- DOBLE SALTO ---
    else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping && !hasDoubleJumped && canDoubleJump) {
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
        if (velocity.y < 0) {  // Si aún está subiendo, forzamos la caída
            velocity.y = 0;   // Reduce la velocidad de subida de golpe
        }
    }

    // --- GRAVEDAD SUAVE Y PROGRESIVA EN LA CAÍDA ---
    if (velocity.y > 0 && !isDashing) {
        velocity.y += std::min(velocity.y * 0.1f, 1.0f);
    }


    // Aplicar la nueva velocidad al jugador
    pbody->body->SetLinearVelocity(velocity);
}

void Player::HandleDash() {
    // Si el dash está desactivado, no hacemos nada
    if (!canDash) return;

    // Obtener la velocidad actual del jugador
    b2Vec2 velocity = pbody->body->GetLinearVelocity();

    //  INICIO DEL DASH 
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_K) == KEY_DOWN && !isDashing) {
        isDashing = true;
        dashRemaining = dashDistance;  //  Establecemos la distancia total del dash

        // Determinar la dirección del dash
        if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
            dashDirection = -1;  // Dash a la izquierda
        }
        else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
            dashDirection = 1;   // Dash a la derecha
        }
        else {
            dashDirection = (velocity.x >= 0) ? 1 : -1;  // Dash en la dirección actual si no hay input
        }
    }

    //  MIENTRAS EL DASH ESTÉ ACTIVO 
    if (isDashing) {
        if (dashRemaining > 0) {
            velocity.x = dashDirection * dashSpeed;
            velocity.y = 0; 
            dashRemaining--;
        }
        else {
            isDashing = false; // Termina el dash
        }
    }

    // Aplicar la nueva velocidad al jugador
    pbody->body->SetLinearVelocity(velocity);
}


