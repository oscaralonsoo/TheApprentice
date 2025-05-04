#pragma once

#include "Input.h"
#include "Render.h"

enum class EntityType
{
	PLAYER,
	BLOODRUSHER,
	CREEBLER,
	MIREBORN,
	CASTOR,
	HYPNOVIPER,
	BROODHEART,
	THUMPOD,
	SCURVER,
	SPEAR,
	LIFE_PLANT,
	CHECKPOINT,
	HELP_ZONE,
	BROOD,
	NULLWARDEN,
	NOCTILUME,
	CAVE_DROP,
	ABILITY_ZONE,
	HIDDEN_ZONE,
	DESTRUCTIBLE_WALL,
	PUSHABLE_BOX,
	DUST_PARTICLE,
	UNKNOWN
};

class PhysBody;

class Entity
{
public:
	Entity(EntityType type) : type(type), active(true) {}

	virtual bool Awake()
	{
		return true;
	}

	virtual bool Start()
	{
		return true;
	}

	virtual bool Update(float dt)
	{
		return true;
	}

	virtual bool PostUpdate()
	{
		return true;
	}

	virtual bool CleanUp()
	{
		return true;
	}

	void Enable()
	{
		if (!active)
		{
			active = true;
			Start();
		}
	}

	void Disable()
	{
		if (active)
		{
			active = false;
			CleanUp();
		}
	}

	virtual void OnCollision(PhysBody* physA, PhysBody* physB) {}

	virtual void OnCollisionEnd(PhysBody* physA, PhysBody* physB) {}

	// Nuevo método para obtener el tipo de entidad
	EntityType GetType() const { return type; }


public:
	std::string name;
	EntityType type;
	bool active = true;

	Vector2D position;
	bool renderable = true;
};