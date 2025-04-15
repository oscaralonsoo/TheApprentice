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
#include "HiddenZone.h"

HiddenZone::HiddenZone() : Entity(EntityType::HIDDEN_ZONE)
{
}

HiddenZone::~HiddenZone() {
}

bool HiddenZone::Awake() {
	return true;
}

bool HiddenZone::Start() {

	//Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor(position.getX(), position.getY(), width, width, bodyType::STATIC);

	//Assign collider type
	pbody->ctype = ColliderType::HIDDEN_ZONE;

	pbody->listener = this;

	return true;
}

bool HiddenZone::Update(float dt)
{
	//Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY());

	return true;
}

bool HiddenZone::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void HiddenZone::OnCollision(PhysBody* physA, PhysBody* physB) {

}

void HiddenZone::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{

}