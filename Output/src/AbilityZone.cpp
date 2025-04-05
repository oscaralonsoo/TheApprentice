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

AbilityZone::AbilityZone() : Entity(EntityType::CAVE_DROP), state(AbilityZoneStates::WAITING)
{
}

AbilityZone::~AbilityZone() {
}

bool AbilityZone::Awake() {
	return true;
}

bool AbilityZone::Start() {
	//Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::KINEMATIC);

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

	if (playerInside)
	{
		// Buscar al jugador sin necesidad de GetPlayer()
		Entity* player = nullptr;
		for (Entity* e : Engine::GetInstance().entityManager->entities)
		{
			if (e->type == EntityType::PLAYER)
			{
				player = e;
				break;
			}
		}

		if (player != nullptr)
		{
			float zoneCenterX = GetPosition().getX();  // centro del collider
			float playerX = player->position.getX();

			float dx = fabs(playerX - zoneCenterX); // distancia en eje X
			float maxDistance = texW / 2.0f; // radio horizontal
			float t = std::min(dx / maxDistance, 1.0f); // normalizado

			float minZoom = 1.0f;
			float maxZoom = 1.4f;
			float zoomValue = maxZoom - (maxZoom - minZoom) * t;

			Engine::GetInstance().render->SetCameraZoom(zoomValue);
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

void AbilityZone::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {
	case ColliderType::PLAYER:	
		printf("ENTRAAAA");
		playerInside = true;
		break;
	default:
		break;
	}
}

void AbilityZone::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype) {
	case ColliderType::PLAYER:
		printf("SALEEE");
		playerInside = false;
		Engine::GetInstance().render->SetCameraZoom(1.0f);
		break;
	default:
		break;
	}
}