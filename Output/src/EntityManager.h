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

	bool PreUpdate(float dt);

	// Called after Awake
	bool Start();

	// Called every frame
	bool Update(float dt);

	// Called after Update
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Additional methods
	Entity* CreateEntity(EntityType type);

	void DestroyEntity(Entity* entity);

	void DestroyAllEntities();

	void AddEntity(Entity* entity);

public:
	std::list<Entity*> entities;

	PhysBody* pbody;

};
