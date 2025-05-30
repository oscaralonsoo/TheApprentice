#pragma once

#include "Input.h"
#include "Render.h"

enum class EntityType
{
	PLAYER,
	BLOODRUSHER,
	CREEBLER,
	PRESSURE_PLATE,
	PRESSURE_DOOR,
	MIREBORN,
	CASTOR,
	SHYVER,
	HYPNOVIPER,
	BROODHEART,
	PERDIZ,
	PANGOLIN,
	LIEBRE,
	ARDILLA,
	BICHOPALO,
	THUMPOD,
	SCURVER,
	SPEAR,
	CRYSTAL,
	BULLET,
	BALL,
	LIFE_PLANT,
	LIFE_PLANT_MAX,
	CHECKPOINT,
	HELP_ZONE,
	BROOD,
	NULLWARDEN,
	NOCTILUME,
	DREADSPIRE,
	MENU_PARTICLE,
	DUNGBEETLE,
	CAVE_DROP,
	ABILITY_ZONE,
	HIDDEN_ZONE,
	DESTRUCTIBLE_WALL,
	PUSHABLE_BOX,
	GEYSER,
	DUST_PARTICLE,
	HOOK_ANCHOR,
	HOOKABLE_BOX,
	STALACTITE,
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

	virtual void SetPhysicsActive(bool active) {}

	// Nuevo método para obtener el tipo de entidad
	EntityType GetType() const { return type; }


public:
	int width = 0;
	int height = 0;
	std::string name;
	EntityType type;
	bool active = true;

	Vector2D position;
	bool renderable = true;
	bool toDelete = false;
};