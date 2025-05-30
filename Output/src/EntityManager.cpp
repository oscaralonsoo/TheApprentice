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
#include "LifePlantMax.h"
#include "PressurePlate.h"
#include "Shyver.h"
#include "PressureDoor.h"
#include "NPC.h"
#include "Hypnoviper.h"
#include "Mireborn.h"
#include "AbilityZone.h"
#include "HiddenZone.h"
#include "Creebler.h"
#include "Scurver.h"
#include "Thumpod.h"
#include "Dreadspire.h"
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
#include "Geyser.h"
#include "DungBeetle.h"
#include "Stalactite.h"

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
	case EntityType::PRESSURE_PLATE:
		entity = new PressurePlate();
		break;
	case EntityType::PRESSURE_DOOR:
		entity = new PressureDoor();
		break;
	case EntityType::SHYVER:
		entity = new Shyver();
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
	case EntityType::DREADSPIRE:
		entity = new Dreadspire();
		break;
	case EntityType::ABILITY_ZONE:
		entity = new AbilityZone();
		break;
	case EntityType::NULLWARDEN:
		entity = new Nullwarden();
		break;
	case EntityType::DUNGBEETLE:
		entity = new DungBeetle();
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
	case EntityType::PERDIZ:
		entity = new NPC(EntityType::PERDIZ);
		break;
	case EntityType::LIEBRE:
		entity = new NPC(EntityType::LIEBRE);
		break;
	case EntityType::PANGOLIN:
		entity = new NPC(EntityType::PANGOLIN);
		break;
	case EntityType::ARDILLA:
		entity = new NPC(EntityType::ARDILLA);
		break;
	case EntityType::BICHOPALO:
		entity = new NPC(EntityType::BICHOPALO);
		break;
	case EntityType::NOCTILUME:
		entity = new Noctilume();
		break;
	case EntityType::LIFE_PLANT:
		entity = new LifePlant();
		break;
	case EntityType::LIFE_PLANT_MAX:
		entity = new LifePlantMax();
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
	case EntityType::GEYSER:
		entity = new Geyser();
		break;
	case EntityType::STALACTITE:
		entity = new Stalactite();
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
	Vector2D camPos = Vector2D(Engine::GetInstance().render->camera.x * -1, Engine::GetInstance().render->camera.y * -1);
	Vector2D camTilePos = Engine::GetInstance().map->WorldToMap(camPos.x, camPos.y);

	int windowWidth, windowHeight;
	SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &windowWidth, &windowHeight);

	Vector2D camSizeWorld = { static_cast<float>(windowWidth), static_cast<float>(windowHeight) };
	Vector2D camSizeTile = Engine::GetInstance().map->WorldToMap(camSizeWorld.x, camSizeWorld.y);

	constexpr int OFFSET_X = 2;
	constexpr int OFFSET_Y = 4;

	if (Engine::GetInstance().menus->currentState != MenusState::GAME || Engine::GetInstance().menus->isPaused)
		return true;

	bool ret = true;

	for (auto entity : entities)
	{
		if (!entity->active) continue;

		Vector2D entityMinWorld = entity->position;
		Vector2D entityMaxWorld = { entity->position.x + entity->width, entity->position.y + entity->height };

		Vector2D entityMinTile = Engine::GetInstance().map->WorldToMap(entityMinWorld.x, entityMinWorld.y);
		Vector2D entityMaxTile = Engine::GetInstance().map->WorldToMap(entityMaxWorld.x, entityMaxWorld.y);

		bool isInCamera =
			entityMaxTile.x >= camTilePos.x - OFFSET_X &&
			entityMinTile.x <= camTilePos.x + camSizeTile.x + OFFSET_X &&
			entityMaxTile.y >= camTilePos.y - OFFSET_Y &&
			entityMinTile.y <= camTilePos.y + camSizeTile.y + OFFSET_Y;

		entity->SetPhysicsActive(isInCamera);

		if (isInCamera)
			ret = entity->Update(dt);

	}

	return ret;
}



bool EntityManager::PostUpdate()
{
	bool ret = true;

	std::vector<Entity*> activeEntities;
	for (auto entity : entities)
	{
		if (entity->active)
			activeEntities.push_back(entity);
	}

	for (auto entity : activeEntities)
	{
		ret = entity->PostUpdate();

		if (entity->toDelete) {
			QueueEntityForDestruction(entity);
		}
	}
	ProcessPendingDestructions();

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
