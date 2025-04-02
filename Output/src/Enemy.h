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

	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;

	//Pathfinding
	bool showPath = false;
	const char* texturePath;

};
