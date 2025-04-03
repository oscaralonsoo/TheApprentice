#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"

struct SDL_Texture;

class Enemy : public Entity
{
public:

	Enemy(EntityType type);
	virtual ~Enemy();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;

		position.x = parameters.attribute("x").as_int();
		position.y = parameters.attribute("y").as_int();
		texW = parameters.attribute("w").as_int();
		texH = parameters.attribute("h").as_int();
		type = parameters.attribute("type").as_string();
		gravity = parameters.attribute("gravity").as_bool();
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	void ResetPath();

	virtual void OnCollision(PhysBody* physA, PhysBody* physB);

	virtual void OnCollisionEnd(PhysBody* physA, PhysBody* physB);


public:
	//Pathfinding
	int steps = 0;
	int maxSteps = 100;

protected:
	Pathfinding* pathfinding;
	PhysBody* pbody;

	SDL_Texture* texture;

	bool gravity;
	std::string type;
	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;

	//Pathfinding
	bool showPath = false;
	const char* texturePath;

};
