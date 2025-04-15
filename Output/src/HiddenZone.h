#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"

struct SDL_Texture;

class HiddenZone : public Entity
{
public:

	HiddenZone();
	virtual ~HiddenZone();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;

		position.x = parameters.attribute("x").as_int();
		position.y = parameters.attribute("y").as_int();
	}

	virtual void OnCollision(PhysBody* physA, PhysBody* physB);

	virtual void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

private:
	PhysBody* pbody;
	int width;
	int height;
	pugi::xml_node parameters;
};
