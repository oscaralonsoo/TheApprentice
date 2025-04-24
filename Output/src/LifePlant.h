#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Physics.h"
#include "Timer.h"
#include "PlayerMechanics.h"

struct SDL_Texture;

enum class LifePlantStates
{
	AVAILABLE,
	CONSUMED
};

class LifePlant : public Entity
{
public:
	LifePlant();
	~LifePlant();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool CleanUp();

	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

private:
	PhysBody* pbody;

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	Animation* currentAnimation = nullptr;
	Animation availableAnim;
	Animation consumedAnim;

	LifePlantStates state = LifePlantStates::AVAILABLE;
};