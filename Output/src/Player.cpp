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
#include "Map.h"
#include "Menus.h"
#include "AbilityZone.h"

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {}

bool Player::Awake() {
	position = Vector2D(1900, 500);
	return true;
}

bool Player::Start() {
	if (initialized) {
		return true;
	}

	texture = Engine::GetInstance().textures->Load(parameters.attribute("texture").as_string());

	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	animation.LoadAnimations(parameters, texture);

    // Create the body at the same position, and ensure it's centered
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX(), (int)position.getY(), 43, 43, bodyType::DYNAMIC, 22, 9);
    pbody->listener = this;
    pbody->ctype = ColliderType::PLAYER;

	mechanics.Init(this);

	initialized = true;

	return true;
}

bool Player::Update(float dt) {
	animation.Update(dt, state, position.getX(), position.getY(), mechanics.IsVisible());

	mechanics.Update(dt);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	// Teclas de debug / efectos visuales
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_1) == KEY_DOWN) {
		mechanics.EnableJump(true);
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_2) == KEY_DOWN) {
		mechanics.EnableDoubleJump(true);
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_3) == KEY_DOWN) {
		mechanics.EnableDash(true);
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_4) == KEY_DOWN) {
		Engine::GetInstance().render->StartCameraShake(5, 100);
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_5) == KEY_DOWN) {
		Engine::GetInstance().render->ToggleCameraLock();
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_6) == KEY_DOWN) {
		Engine::GetInstance().render->ToggleVerticalOffsetLock();
	}

	return true;
}

bool Player::CleanUp() {
	LOG("Cleanup player");
	Engine::GetInstance().textures->UnLoad(texture);
	return true;
}

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {
	case ColliderType::DOOR:
		LOG("Collision DOOR");
		targetScene = physB->targetScene;

		Engine::GetInstance().scene->newPosition.x = physB->playerPosX;
		Engine::GetInstance().scene->newPosition.y = physB->playerPosY;

		Engine::GetInstance().scene.get()->StartTransition(targetScene);
		break;
	default:
		mechanics.OnCollision(physA, physB);
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
	mechanics.OnCollisionEnd(physA, physB);
	switch (physB->ctype) {
	case ColliderType::ABILITY_ZONE:
		Engine::GetInstance().render.get()->SetCameraZoom(1.0f);
		printf("SALEEEEEEEE");
		break;
	default:
		mechanics.OnCollision(physA, physB);
		break;
	}
}

void Player::SetPosition(Vector2D pos) {
	if (!pbody) {
		printf("ERROR: pbody es nullptr\n");
		return;
	}

	if (!pbody->body) {
		printf("ERROR: pbody->body es nullptr\n");
		return;
	}

	printf("SetPosition a: X = %.2f, Y = %.2f\n", pos.getX(), pos.getY());

	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}


Vector2D Player::GetPosition() const {
	return Vector2D(position.getX() + texW / 2, position.getY() + texH / 2);
}

int Player::GetMovementDirection() const {
	return mechanics.GetMovementDirection();
}

int Player::GetTextureWidth() const {
	return texW;  // Asegúrate de tener texW bien definido (el ancho del sprite)
}