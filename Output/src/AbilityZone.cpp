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

	currentAnimation = &idleAnim;

	//Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW, texH, bodyType::STATIC);

	//Assign collider type
	pbody->ctype = ColliderType::ABILITY_ZONE;

	pbody->listener = this;

	return true;
}

bool AbilityZone::Update(float dt)
{
	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.  
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);


	Player* player = Engine::GetInstance().scene->GetPlayer();
	PlayerMechanics* mechanics = player->GetMechanics();

	if (playerInside)
	{
		float centerX = position.getX() + texW / 2;
		Vector2D currentPos = player->GetPosition();
		float playerCenterX = currentPos.getX() + player->GetTextureWidth() / 2;

		float distance = abs(playerCenterX - centerX);
		float maxDistance = texW / 2.0f;
		float t = 1.0f - std::min(distance / maxDistance, 1.0f); // 0 lejos, 1 en el centro

		// Zoom entre 1.0 y 1.3
		float zoom = 1.0f + t * 0.3f;
		Engine::GetInstance().render.get()->SetCameraZoom(zoom);

		// Frenado progresivo
		b2Vec2 velocity = player->pbody->body->GetLinearVelocity();
		velocity.x *= (1.0f - t);  // 1 lejos = velocidad normal, 0 cerca = velocidad cero
		player->pbody->body->SetLinearVelocity(velocity);

		// Cuando esté muy cerca, bloquear completamente
		if (distance < 20.0f)
		{
			velocity.x = 0.0f;
			player->pbody->body->SetLinearVelocity(velocity);
			mechanics->cantMove = true;
			if (playerInsideJump)
			{
				if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
					Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
					mechanics->cantMove = false;
					mechanics->EnableJump(true);
				}
			}
			else if (playerInsideDoubleJump)
			{
				if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
					Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
					mechanics->cantMove = false;
					mechanics->EnableDoubleJump(true);
				}
			}
			else if (playerInsideDash)
			{
				if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
					Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
					mechanics->cantMove = false;
					mechanics->EnableDash(true);
				}
			}
		}
	}

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY());

	return true;
}

bool AbilityZone::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
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

void AbilityZone::OnCollision(PhysBody* physA, PhysBody* physB){
	Player* player = Engine::GetInstance().scene->GetPlayer();
	PlayerMechanics* mechanics = player->GetMechanics();
	switch (physB->ctype) {
	case ColliderType::PLAYER:
		playerInside = true;
		mechanics->canAttack = false;
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

void AbilityZone::OnCollisionEnd(PhysBody* physA, PhysBody* physB){
	Player* player = Engine::GetInstance().scene->GetPlayer();
	PlayerMechanics* mechanics = player->GetMechanics();
	switch (physB->ctype) {
	case ColliderType::PLAYER:
		playerInside = false;
		mechanics->canAttack = true;
		playerInsideJump = false;
		playerInsideDoubleJump = false;
		playerInsideDash = false;
		Engine::GetInstance().render.get()->SetCameraZoom(1.0f);
		break;
	default:
		break;
	}
}