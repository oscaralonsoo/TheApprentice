#include "EntityManager.h"
#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Scene.h"
#include "Log.h"
#include "CaveDrop.h"
#include "Bloodrusher.h"
#include "Hypnoviper.h"
#include "Mireborn.h"
#include "AbilityZone.h"
#include "Thumpod.h"
#include "Brood.h"
#include "Broodheart.h"

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
	case EntityType::PLAYER:
		entity = new Player();
		break;
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
	default:
		break;
	}

	entities.push_back(entity);

	return entity;
}

// Function to destroy a specific Entity
void EntityManager::DestroyEntity(Entity* entity)
{
	for (auto it = entities.begin(); it != entities.end(); ++it)
	{
		if (*it == entity) {
			(*it)->CleanUp();
			delete* it; // Free the allocated memory
			entities.erase(it); // Remove the entity from the list
			break; // Exit the loop after removing the entity
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
	if (Engine::GetInstance().menus->currentState != MenusState::GAME|| Engine::GetInstance().menus->isPaused)
		return true;

	bool ret = true;
	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->Update(dt);
	}
	return ret;
}

bool EntityManager::PostUpdate()
{
	bool ret = true;
	for (const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->PostUpdate();
	}
	return ret;
}
