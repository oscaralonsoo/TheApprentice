#include "EntityManager.h"
#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Scene.h"
#include "Log.h"
#include "CaveDrop.h"
#include "Bloodrusher.h"
#include "Nullwarden.h"
#include "LifePlant.h"
#include "NPC.h"
#include "Hypnoviper.h"
#include "Mireborn.h"
#include "AbilityZone.h"
#include "HiddenZone.h"
#include "Creebler.h"
#include "Scurver.h"
#include "Thumpod.h"
#include "Brood.h"
#include "Broodheart.h"
#include "DestructibleWall.h"
#include "PushableBox.h"
#include "AbilityZone.h"
#include "Noctilume.h"
#include "HelpZone.h"
#include "Checkpoint.h"
#include "HookAnchor.h"
#include "HokableBox.h"

EntityManager::EntityManager() : Module()
{
	name = "entitymanager";
}

// Destructor
EntityManager::~EntityManager()
{}

// Called before render is available
bool EntityManager::Awake()
{
	LOG("Loading Entity Manager");
	bool ret = true;

	//Iterates over the entities and calls the Awake
	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->Awake();
	}

	return ret;

}

bool EntityManager::Start() {

	bool ret = true; 

	//Iterates over the entities and calls Start
	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->Start();
	}

	return ret;
}

// Called before quitting
bool EntityManager::CleanUp()
{
	bool ret = true;

	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->CleanUp();
	}

	entities.clear();

	return ret;
}

Entity* EntityManager::CreateEntity(EntityType type)
{
	Entity* entity = nullptr; 

	//L04: TODO 3a: Instantiate entity according to the type and add the new entity to the list of Entities
	switch (type)
	{
	case EntityType::CAVE_DROP:
		entity = new CaveDrop();
		break;
	case EntityType::BLOODRUSHER:
		entity = new Bloodrusher();
		break;
	case EntityType::HYPNOVIPER:
		entity = new Hypnoviper();
		break;
	case EntityType::THUMPOD:
		entity = new Thumpod();
		break;
	case EntityType::CREEBLER:
		entity = new Creebler();
		break;
	case EntityType::SCURVER:
		entity = new Scurver();
		break;
	case EntityType::MIREBORN:
		entity = new Mireborn();
		break;
	case EntityType::BROODHEART:
		entity = new Broodheart();
		break;
	case EntityType::BROOD:
		entity = new Brood();
		break;
	case EntityType::ABILITY_ZONE:
		entity = new AbilityZone();
		break;
	case EntityType::NULLWARDEN:
		entity = new Nullwarden();
		break;
	case EntityType::HIDDEN_ZONE:
		entity = new HiddenZone();
		break;
	case EntityType::DESTRUCTIBLE_WALL:
		entity = new DestructibleWall();
		break;
	case EntityType::PUSHABLE_BOX:
		entity = new PushableBox();
		break;
	case EntityType::CASTOR:
		entity = new NPC(EntityType::CASTOR);
		break;
	case EntityType::NOCTILUME:
		entity = new Noctilume();
		break;
	case EntityType::LIFE_PLANT:
		entity = new LifePlant();
		break;
	case EntityType::HELP_ZONE:
		entity = new HelpZone();
		break;
	case EntityType::CHECKPOINT:
		entity = new Checkpoint();
		break;
	case EntityType::PLAYER:
		entity = new Player();
		break;
	case EntityType::HOOK_ANCHOR:
		entity = new HookAnchor();
		break;
	case EntityType::HOOKABLE_BOX:
		return new HookableBox();
	default:
		break;
	}

	entities.push_back(entity);

	return entity;
}

// Function to destroy a specific Entity
void EntityManager::DestroyEntity(Entity* entity)
{
	for (auto it = entities.begin(); it != entities.end();)
	{
		if (*it == entity) {
			(*it)->CleanUp();
			delete* it;
			it = entities.erase(it);
			break;
		}
		else {
			++it;
		}
	}
}

// Function to destroy All Entities & Not The Player
void EntityManager::DestroyAllEntities() {

	for (auto it = entities.begin(); it != entities.end(); ) {
		if ((*it)->type != EntityType::PLAYER) {
			LOG("Destroying entity at position: (%f, %f)", (*it)->position.getX(), (*it)->position.getY());
			(*it)->CleanUp();
			delete* it;
			it = entities.erase(it);
		}
		else {
			++it;
		}
	}
	LOG("All entities removed.");
}

// Function to add a Specific Entity
void EntityManager::AddEntity(Entity* entity)
{
	if ( entity != nullptr) entities.push_back(entity);
}

bool EntityManager::Update(float dt)
{
	if (Engine::GetInstance().menus->currentState != MenusState::GAME|| Engine::GetInstance().menus->isPaused /*TODO JAVI-- - SI VIDAS = 0, NO SE DIBUJA NINGUNA ENTIDAD*/)
		return true;

	bool ret = true;

	std::vector<Entity*> activeEntities;
	for (auto entity : entities)
	{
		if (entity->active)
			activeEntities.push_back(entity);
	}


	for (auto entity : activeEntities)
	{
		ret = entity->Update(dt);
	}

	return ret;
}

bool EntityManager::PostUpdate()
{
	bool ret = true;

	// Copia segura de las entidades activas
	std::vector<Entity*> activeEntities;
	for (auto entity : entities)
	{
		if (entity->active)
			activeEntities.push_back(entity);
	}

	// Ahora iteramos sobre la copia, aunque la original se modifique
	for (auto entity : activeEntities)
	{
		ret = entity->PostUpdate();
	}

	return ret;
}
void EntityManager::QueueEntityForDestruction(Entity* entity) {
	pendingDestroy.push_back(entity);
}

void EntityManager::ProcessPendingDestructions() {
	for (Entity* e : pendingDestroy) {
		DestroyEntity(e); 
	}
	pendingDestroy.clear();
}
