#pragma once

#include "Module.h"
#include "Entity.h"
#include <list>

class ParticleManager : public Module
{
public:

	ParticleManager();

	// Destructor
	virtual ~ParticleManager();

	// Called before render is available
	bool Awake();

	// Called after Awake
	bool Start();

	// Called every frame
	bool Update(float dt);

	// Called after Update
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Additional methods
	Entity* CreateParticle(EntityType type);

	void DestroyParticle(Entity* particle);

	void DestroyAllParticles();

	void SpawnDustParticles();

	void SetParticlesByMap(int scene);

public:
	std::list<Entity*> particles;

	PhysBody* pbody;

};
