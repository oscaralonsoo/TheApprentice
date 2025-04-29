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
		CATEGORY_PLAYER          // Solo interactúa con el player
	);

	//Assign collider type
	pbody->ctype = ColliderType::ABILITY_ZONE;

	pbody->listener = this;

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
		VignetteChange(dt);

		float rightLimit = position.getX() + texW;
		float targetStopX = rightLimit - 120.0f;
		float playerRight = player->GetPosition().getX() + player->GetTextureWidth();
		float distance = abs(playerRight - targetStopX);

		if (distance <= 2.0f) {
			b2Vec2 stopVelocity = player->pbody->body->GetLinearVelocity();
			stopVelocity.x = 0.0f;
			player->pbody->body->SetLinearVelocity(stopVelocity);
			player->GetMechanics()->GetMovementHandler()->SetCantMove(true);
		}
		else {
			float maxDistance = pbody->width * 1.5f; // empieza a frenar antes si el collider es más ancho
			float t = 1.0f - std::min(distance / maxDistance, 1.0f);
			b2Vec2 velocity = player->pbody->body->GetLinearVelocity();
			float slowdownFactor = std::max(0.3f, pow(1.0f - t, 1.0f));
			velocity.x *= slowdownFactor;
			player->pbody->body->SetLinearVelocity(velocity);
		}

		if (playerRight >= rightLimit - 144.0f) {
			if (playerInsideJump && Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
				Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
				player->GetMechanics()->GetMovementHandler()->SetCantMove(true);
				mechanics->EnableJump(true);
				player->GetMechanics()->GetMovementHandler()->SetCanAttack(true);
				mechanics->GetHealthSystem()->SetVignetteSize(Engine::GetInstance().scene->previousVignetteSize);
				markedForDeletion = true;
				Engine::GetInstance().menus->abilityName = "jump";
				Engine::GetInstance().menus->StartTransition(false, MenusState::ABILITIES);
			}
			else if (playerInsideDoubleJump && Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
				Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
				player->GetMechanics()->GetMovementHandler()->SetCantMove(true);
				mechanics->EnableDoubleJump(true);
				player->GetMechanics()->GetMovementHandler()->SetCanAttack(true);
				mechanics->GetHealthSystem()->SetVignetteSize(Engine::GetInstance().scene->previousVignetteSize);
				markedForDeletion = true;
				Engine::GetInstance().menus->abilityName = "doublejump";
				Engine::GetInstance().menus->StartTransition(false, MenusState::ABILITIES);
			}
			else if (playerInsideDash && Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
				Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
				player->GetMechanics()->GetMovementHandler()->SetCantMove(true);
				mechanics->EnableDash(true);
				player->GetMechanics()->GetMovementHandler()->SetCanAttack(true);
				mechanics->GetHealthSystem()->SetVignetteSize(Engine::GetInstance().scene->previousVignetteSize);
				markedForDeletion = true;
				Engine::GetInstance().menus->StartTransition(false, MenusState::ABILITIES);
				Engine::GetInstance().menus->abilityName = "dash";
			}
		}
	}

	if (abilitySprite) {
		int drawX = position.getX() + texW - abilitySpriteW - 100;
		int drawY = position.getY() + texH / 2 - abilitySpriteH / 2 + 40;
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

	int minVignetteSize = 300;  int maxVignetteSize = 900;
	int newVignetteSize = minVignetteSize + static_cast<int>((maxVignetteSize - minVignetteSize) * progress);

	// Añadir efecto de vibración
	if (progress > 0.95f) {
		float vibrateAmplitude = 200.0f;
		float vibrateSpeed = 30.0f;
		float offset = sinf(dt * vibrateSpeed) * vibrateAmplitude;
		newVignetteSize += static_cast<int>(offset);
	}
	// Aplicar tamaño
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