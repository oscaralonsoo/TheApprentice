#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Map.h"
#include "Physics.h"
#include "Module.h"
#include "AbilityZone.h"
#include "Player.h"
#include "PlayerMechanics.h"
#include "Scene.h"

template<typename T>
T clamp(const T& value, const T& min, const T& max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

AbilityZone::AbilityZone() : Entity(EntityType::CAVE_DROP), state(AbilityZoneStates::WAITING)
{
}

AbilityZone::~AbilityZone() {
}

bool AbilityZone::Awake() {
	return true;
}

bool AbilityZone::Start() {
	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	for (pugi::xml_node enemyNode = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		if (std::string(enemyNode.attribute("type").as_string()) == type)
		{
			texture = Engine::GetInstance().textures.get()->Load(enemyNode.attribute("texture").as_string());
			idleAnim.LoadAnimations(enemyNode.child("idle"));
		}
	}

	pugi::xml_node abilitiesNode = loadFile.child("config").child("scene").child("animations").child("abilities");

	for (pugi::xml_node abilityNode = abilitiesNode.child("ability"); abilityNode; abilityNode = abilityNode.next_sibling("ability")) {
		if (std::string(abilityNode.attribute("type").as_string()) == type) {
			std::string texPath = abilityNode.attribute("texture").as_string();
			abilitySprite = Engine::GetInstance().textures->Load(texPath.c_str());
			abilitySpriteW = abilityNode.attribute("w").as_int();
			abilitySpriteH = abilityNode.attribute("h").as_int();
			break;
		}
	}

	currentAnimation = &idleAnim;

	//Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor(
		(int)position.getX() + texH / 2,
		(int)position.getY() + texH / 2,
		texW, texH,
		bodyType::STATIC,
		CATEGORY_ABILITY_ZONE,   // Este sensor es de tipo "AbilityZone"
		CATEGORY_PLAYER          // Solo interact�a con el player
	);

	//Assign collider type
	pbody->ctype = ColliderType::ABILITY_ZONE;

	pbody->listener = this;

	jumpSprite = Engine::GetInstance().textures->Load("Assets/Props/huevosrana.png");
	doubleJumpSprite = Engine::GetInstance().textures->Load("textures/doublejump_icon.png");
	dashSprite = Engine::GetInstance().textures->Load("Assets/Props/Dash_Icon.png");
	hookSprite = Engine::GetInstance().textures->Load("Assets/Props/araña.png");
	glideSprite = Engine::GetInstance().textures->Load("Assets/Props/Glide_Icon.png");
	wallJumpSprite = Engine::GetInstance().textures->Load("Assets/Props/WallJump_Icon.png");
	pushSprite = Engine::GetInstance().textures->Load("Assets/Props/Push_Icon.png");

	// Obtener el controller directamente desde el MovementHandler del jugador
	Player* player = Engine::GetInstance().scene->GetPlayer();
	if (player) {
		PlayerMechanics* mechanics = player->GetMechanics();
		if (mechanics) {
			SDL_GameController* controller = mechanics->GetMovementHandler()->GetController();
			if (controller) {
				SetController(controller);
				LOG("AbilityZone: controller obtenido directamente en Start");
			}
			else {
				LOG("AbilityZone: no se pudo obtener el controller desde MovementHandler");
			}
		}
	}

	return true;
}

bool AbilityZone::PreUpdate() {

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY());
	return true;
}

bool AbilityZone::Update(float dt)
{
	if (!pbody) return true;

	Player* player = Engine::GetInstance().scene->GetPlayer();
	PlayerMechanics* mechanics = player->GetMechanics();

	if (playerInside)
	{
		// TODO JAVI --- PLAYER CANNOT JUMP IF INSIDE ABILITY ZONE
		VignetteChange(dt);

		float rightLimit = position.getX() + texW;
		float targetStopX = rightLimit - 120.0f;
		float playerRight = player->GetPosition().getX() + player->GetTextureWidth();
		float distance = abs(playerRight - targetStopX);
		player->GetMechanics()->GetMovementHandler()->SetCanAttack(true);

		if (distance <= 2.0f) {
			b2Vec2 stopVelocity = player->pbody->body->GetLinearVelocity();
			stopVelocity.x = 0.0f;
			player->pbody->body->SetLinearVelocity(stopVelocity);
			player->GetMechanics()->GetMovementHandler()->SetCantMove(true);
		}
		else {
			float maxDistance = pbody->width * 1.5f; // empieza a frenar antes si el collider es m�s ancho
			float t = 1.0f - std::min(distance / maxDistance, 1.0f);
			b2Vec2 velocity = player->pbody->body->GetLinearVelocity();
			float slowdownFactor = std::max(0.3f, pow(1.0f - t, 1.0f));
			velocity.x *= slowdownFactor;
			player->pbody->body->SetLinearVelocity(velocity);
		}

		if (playerRight >= rightLimit - 144.0f) {
			bool confirmPressed = false;

			// J del teclado
			if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
				confirmPressed = true;
			}

			if (controller && SDL_GameControllerGetAttached(controller)) {
				bool r1Now = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);

				LOG("AbilityZone: mando conectado: %s", SDL_GameControllerName(controller));
				LOG("AbilityZone: botón R1 estado actual: %d", r1Now);

				if (r1Now && !xHeld) {
					LOG("AbilityZone: botón R1 pulsado");
					confirmPressed = true;
				}
				xHeld = r1Now;
			}
			else {
				LOG("AbilityZone: mando no conectado o no válido");
			}

			if (confirmPressed) {
				player->GetAnimation()->SetStateIfHigherPriority("eat"); 

				Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
				player->GetMechanics()->GetMovementHandler()->SetCantMove(true);
				player->GetMechanics()->GetMovementHandler()->SetCanAttack(true);
				mechanics->GetHealthSystem()->SetVignetteSize(Engine::GetInstance().scene->previousVignetteSize);
				markedForDeletion = true;

				if (type == "Jump") {
					mechanics->EnableJump(true);
					Engine::GetInstance().menus->abilityName = "jump";
				}
				else if (type == "DoubleJump") {
					mechanics->EnableDoubleJump(true);
					Engine::GetInstance().menus->abilityName = "doublejump";
				}
				else if (type == "Dash") {
					mechanics->EnableDash(true);
					Engine::GetInstance().menus->abilityName = "dash";
				}
				else if (type == "WallJump") {
					mechanics->EnableDash(true);
					Engine::GetInstance().menus->abilityName = "walljump";
				}
				else if (type == "Glide") {
					mechanics->EnableGlide(true);
					Engine::GetInstance().menus->abilityName = "glide";
				}
				else if (type == "Hook") {
					mechanics->GetMovementHandler()->SetHookUnlocked(true);
					Engine::GetInstance().menus->abilityName = "hook";
				}
				else if (type == "Glide") {
					mechanics->EnableGlide(true);
					Engine::GetInstance().menus->abilityName = "glide";
				}
				else if (type == "WallJump") {
					mechanics->EnableWallJump(true);
					Engine::GetInstance().menus->abilityName = "walljump";
				}
				else if (type == "Push") {
					mechanics->EnablePush(true);
					Engine::GetInstance().menus->abilityName = "push";
				}

				player->GetMechanics()->GetMovementHandler()->SetCantMove(false);
				player->GetMechanics()->GetMovementHandler()->disableAbilities = false;
				Engine::GetInstance().menus->StartTransition(false, MenusState::ABILITIES);
			}
		}
	}

	int drawX = position.getX() + texW - abilitySpriteW - 100;
	int drawY = position.getY() + texH / 2 - abilitySpriteH / 2 + 20;

	bool drawn = false;

	if (type == "Jump" && jumpSprite) {
		int drawX = position.getX() + texW - abilitySpriteW - 100;
		int drawY = position.getY() + texH / 2 - abilitySpriteH / 2 + 20;
		Engine::GetInstance().render->DrawTexture(jumpSprite, drawX, drawY);
		drawn = true;
	}
	else if (type == "DoubleJump" && doubleJumpSprite) {
		int drawX = position.getX() + texW - abilitySpriteW - 100;
		int drawY = position.getY() + texH / 2 - abilitySpriteH / 2 + 20;
		Engine::GetInstance().render->DrawTexture(doubleJumpSprite, drawX, drawY);
		drawn = true;
	}
	else if (type == "Dash" && dashSprite) {
		int drawX = position.getX() + texW - abilitySpriteW - 100;
		int drawY = position.getY() + texH / 2 - abilitySpriteH / 2 + 20;
		Engine::GetInstance().render->DrawTexture(dashSprite, drawX, drawY);
		drawn = true;
	}
	else if (type == "Hook" && hookSprite) {
		int drawX = position.getX() + texW - abilitySpriteW - 200;
		int drawY = position.getY() + texH / 2 - abilitySpriteH / 2 - 50;
		Engine::GetInstance().render->DrawTexture(hookSprite, drawX, drawY);
		drawn = true;
	}
	else if (type == "Glide" && glideSprite) {
		int drawX = position.getX() + texW - abilitySpriteW - 200;
		int drawY = position.getY() + texH / 2 - abilitySpriteH / 2 - 50;
		Engine::GetInstance().render->DrawTexture(glideSprite, drawX, drawY);
		drawn = true;
	}
	else if (type == "WallJump" && wallJumpSprite) {
		int drawX = position.getX() + texW - abilitySpriteW - 400;
		int drawY = position.getY() + texH / 2 - abilitySpriteH / 2 - 350;
		Engine::GetInstance().render->DrawTexture(wallJumpSprite, drawX, drawY);
		drawn = true;
	}
	else if (type == "Push" && pushSprite) {
		int drawX = position.getX() + texW - abilitySpriteW - 250;
		int drawY = position.getY() + texH / 2 - abilitySpriteH / 2 - 125;
		Engine::GetInstance().render->DrawTexture(pushSprite, drawX, drawY);
		drawn = true;
	}

	if (!drawn && abilitySprite) {
		Engine::GetInstance().render->DrawTexture(abilitySprite, drawX, drawY);
	}

	return true;
}

bool AbilityZone::PostUpdate() {
	if (markedForDeletion) {
		Engine::GetInstance().entityManager.get()->DestroyEntity(this);
	}

	return true;
}

bool AbilityZone::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void AbilityZone::VignetteChange(float dt)
{
	Player* player = Engine::GetInstance().scene->GetPlayer();
	PlayerMechanics* mechanics = player->GetMechanics();

	float zoneStartX = position.getX();
	float zoneEndX = zoneStartX + texW;
	float playerX = player->GetPosition().getX();

	float progress = (playerX - zoneStartX) / (zoneEndX - zoneStartX);
	progress = clamp(progress, 0.0f, 1.0f);
	progress = 1.0f - powf(1.0f - progress, 4.0f);

	newVignetteSize = minVignetteSize + static_cast<int>((maxVignetteSize - minVignetteSize) * progress);

	// A�adir efecto de vibraci�n
	if (progress > 0.95f) {
		float offset = sinf(dt * vibrateSpeed) * vibrateAmplitude;
		newVignetteSize += static_cast<int>(offset);
	}
	// Aplicar tama�o
	mechanics->GetHealthSystem()->SetVignetteSize(newVignetteSize);
}

void AbilityZone::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D AbilityZone::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void AbilityZone::OnCollision(PhysBody* physA, PhysBody* physB) {
	Player* player = Engine::GetInstance().scene->GetPlayer();
	PlayerMechanics* mechanics = player->GetMechanics();
	switch (physB->ctype) {
	case ColliderType::PLAYER:
		playerInside = true;
		player->GetMechanics()->GetMovementHandler()->SetCanAttack(false);
		Engine::GetInstance().scene->previousVignetteSize = mechanics->GetHealthSystem()->GetVignetteSize();
		player->GetMechanics()->GetMovementHandler()->disableAbilities = true;
		if (type == "Jump") {
			playerInsideJump = true;
		}
		else if (type == "DoubleJump") {
			playerInsideDoubleJump = true;
		}
		else if (type == "Dash") {
			playerInsideDash = true;
		}
		break;
	default:
		break;
	}
}

void AbilityZone::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
	Player* player = Engine::GetInstance().scene->GetPlayer();
	PlayerMechanics* mechanics = player->GetMechanics();
	switch (physB->ctype) {
	case ColliderType::PLAYER:
		playerInside = false;
		player->GetMechanics()->GetMovementHandler()->SetCanAttack(true);
		playerInsideJump = false;
		playerInsideDoubleJump = false;
		playerInsideDash = false;
		mechanics->GetHealthSystem()->SetVignetteSize(Engine::GetInstance().scene->previousVignetteSize);
		break;
	default:
		break;
	}
}

void AbilityZone::SetController(SDL_GameController* controller) {
	this->controller = controller;
	LOG("AbilityZone: controller asignado correctamente: %p", controller);
}