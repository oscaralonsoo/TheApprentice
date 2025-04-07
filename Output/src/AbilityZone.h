#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"

struct SDL_Texture;

enum class AbilityZoneStates
{
	WAITING,
	ACTIVATED,
	UNLOCKED
};

class AbilityZone : public Entity
{
public:

	AbilityZone();
	virtual ~AbilityZone();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;

		position.x = parameters.attribute("x").as_int();
		position.y = parameters.attribute("y").as_int();
		type = parameters.attribute("type").as_string();
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	virtual void OnCollision(PhysBody* physA, PhysBody* physB);

	virtual void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

private:
	PhysBody* pbody;
	SDL_Texture* texture;
	std::string type;
	Animation idleAnim;
	Animation* currentAnimation = nullptr;
	int texW = 50;
	int texH = 50;
	pugi::xml_node parameters;
	AbilityZoneStates state = AbilityZoneStates::WAITING;
};
