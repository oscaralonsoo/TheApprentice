#pragma once

#include "Module.h"
#include "Entity.h"
#include <list>
#include "Enemy.h"

class EntityManager : public Module
{
public:

	EntityManager();

	// Destructor
	virtual ~EntityManager();

	// Called before render is available
	bool Awake();

	// Called after Awake
	bool Start();

	// Called every frame
	bool Update(float dt);

	// Called after Update
	bool PostUpdate();

	void QueueEntityForDestruction(Entity* entity);

	void ProcessPendingDestructions();

	// Called before quitting
	bool CleanUp();

	// Additional methods
	Entity* CreateEntity(EntityType type);

	void DestroyEntity(Entity* entity);

	void DestroyAllEntities();

	void AddEntity(Entity* entity);

	const std::list<Entity*>& GetEntities() const { return entities; }


public:
	std::list<Entity*> entities;
	std::vector<Entity*> pendingDestroy;
	PhysBody* pbody;

};
