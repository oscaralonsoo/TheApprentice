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
		state = "run_left";
	}
	else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
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

    // Si el jugador presiona la tecla de salto y no está en el aire
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping) {
        velocity.y = -jumpForce;  // Aplicamos fuerza hacia arriba
        isJumping = true;         // Marcamos que el jugador está en el aire
    }

    // Aplicamos la nueva velocidad
    pbody->body->SetLinearVelocity(velocity);
}


