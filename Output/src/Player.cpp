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
	position = Vector2D(1088, 960);
	return true;
}

bool Player::Start() {
	animation = new PlayerAnimation();
	if (initialized) {
		return true;
	}

	texture = Engine::GetInstance().textures->Load(parameters.attribute("texture").as_string());

	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	animation->LoadAnimations(parameters, texture);
	animation->SetPlayer(this);

    // Create the body at the same position, and ensure it's centered
	pbody = Engine::GetInstance().physics.get()->CreateRectangle(
		(int)position.getX(),
		(int)position.getY(),
		60, 40,
		bodyType::DYNAMIC,
		0, 5,
		CATEGORY_PLAYER,
		CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_SPIKE | CATEGORY_ENEMY | CATEGORY_SAVEGAME | CATEGORY_DOWN_CAMERA | CATEGORY_ABILITY_ZONE | CATEGORY_HIDDEN_ZONE | CATEGORY_NPC | CATEGORY_LIFE_PLANT | CATEGORY_DOOR | CATEGORY_GEYSER
	);
	pbody->listener = this;
	pbody->ctype = ColliderType::PLAYER;

	enemySensor = Engine::GetInstance().physics->CreateRectangleSensor(
		(int)position.getX(),
		(int)position.getY(),
		45, 40,
		bodyType::DYNAMIC,
		CATEGORY_PLAYER_DAMAGE,
		CATEGORY_ENEMY                  
	);
	enemySensor->ctype = ColliderType::PLAYER_DAMAGE;
	enemySensor->listener = this;

	mechanics.Init(this);

	initialized = true;

	pbody->body->SetGravityScale(2.0f);

	positionArray = {
	Vector2D(1461, 753),
	Vector2D(1082, 908),
	Vector2D(760, 3826),
	Vector2D(448, 1856),
	Vector2D(3402, 4020),
	Vector2D(3392, 6080),
	Vector2D(428, 620),
	Vector2D(1152, 1088),
	Vector2D(276, 2824),
	Vector2D(7038, 446),
	Vector2D(4372, 1150),
	Vector2D(7939, 4656),
	Vector2D(5578, 4486),
	Vector2D(1510, 732),
	Vector2D(449, 932),
	Vector2D(446, 928),
	};

	intArray = {0, 1, 12, 14, 21, 22, 23, 31, 41, 42, 43, 51, 52, 69, 99, 6666};

	currentIndex = 1;

	return true;
}

bool Player::Update(float dt) {
	bool flip = mechanics.GetMovementDirection() < 0;

	if (GetState() == "hook" && pbody && pbody->body) {
		float vx = pbody->body->GetLinearVelocity().x;
		if (vx < -0.1f) flip = true;
		else if (vx > 0.1f) flip = false;
	}

	if (pbody == nullptr || pbody->body == nullptr) {
		LOG("ERROR: pbody o body es nullptr");
		return false;  // o gesti�n espec�fica
	}

	if (enemySensor && enemySensor->body) {
		b2Vec2 mainPos = pbody->body->GetPosition();
		enemySensor->body->SetTransform(mainPos, 0);
	}
	
	animation->Update(state, position.getX(), position.getY() - 5, mechanics.IsVisible(), flip);
	mechanics.Update(dt);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	// Teclas de debug / efectos visuales
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F4) == KEY_DOWN) {
		mechanics.GetHealthSystem()->AddLife();
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) {
		mechanics.GetHealthSystem()->AddMaxLife();
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) {
		mechanics.GetHealthSystem()->TakeDamage();
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) {
		mechanics.ToggleGodMode();
		if (mechanics.IsGodMode()) {
			pbody->body->SetGravityScale(0.0f);
		}
		else {
			pbody->body->SetGravityScale(2.0f);
		}
	}
	// Cambio de escena con F8 (avanzar)
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN) {
		if (currentIndex + 1 < intArray.size()) {
			currentIndex++;
			int nextScene = intArray[currentIndex];
			Vector2D pos = positionArray[currentIndex];

			Engine::GetInstance().scene->newPosition = pos;
			Engine::GetInstance().scene->StartTransition(nextScene);
		}
	}

	// Cambio de escena con F7 (retroceder)
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F7) == KEY_DOWN) {
		if (currentIndex - 1 >= 0) {
			currentIndex--;
			int nextScene = intArray[currentIndex];
			Vector2D pos = positionArray[currentIndex];

			Engine::GetInstance().scene->newPosition = pos;
			Engine::GetInstance().scene->StartTransition(nextScene);
		}
	}


	if (enemySensor && enemySensor->body) {
		b2Vec2 mainPos = pbody->body->GetPosition();
		enemySensor->body->SetTransform(mainPos, 0);
	}

	return true;
}

bool Player::PostUpdate() {
	bool flip = mechanics.GetMovementDirection() < 0;

	// Si est� atacando, forzar el flip a la direcci�n guardada del ataque
	if (mechanics.GetMovementHandler()->GetAttackMechanic().IsAttacking()) {
		flip = mechanics.GetMovementHandler()->GetAttackMechanic().attackFlip;
	}
	// Forzar flip del wall slide si est� deslizando por la pared
	else if (mechanics.GetMovementHandler()->IsWallSliding()) {
		flip = mechanics.GetMovementHandler()->wallSlideFlip;
	}

	mechanics.PostUpdate();
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
		break;
	default:
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
	return texW; 
}

bool Player::IsTouchingPlatform() const {
	for (b2ContactEdge* edge = pbody->body->GetContactList(); edge; edge = edge->next) {
		b2Contact* contact = edge->contact;
		if (contact->IsTouching()) {			
			if (contact->GetFixtureA()->GetFilterData().maskBits & CATEGORY_PLAYER)
			{
				if (contact->GetFixtureB()->GetFilterData().maskBits & CATEGORY_PLATFORM) {
					return true;
				}
			}
			else
			{
				if (contact->GetFixtureA()->GetFilterData().maskBits & CATEGORY_PLATFORM) {
					return true;
				}
			}
		}
	}
	return false;
}