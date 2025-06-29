#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Physics.h"
#include "Timer.h"
#include "PlayerMechanics.h"

struct SDL_Texture;

enum class LifePlantMaxStates
{
	AVAILABLE,
	CONSUMED
};

class LifePlantMax : public Entity
{
public:
	LifePlantMax();
	~LifePlantMax();

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
	PlayerMechanics mechanics;
	LifePlantMaxStates state = LifePlantMaxStates::AVAILABLE;
	int eatSound = 0;
	int soundInteractId = 0;
};