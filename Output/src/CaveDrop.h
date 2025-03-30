#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"

struct SDL_Texture;

class CaveDrop : public Entity
{
public:

	CaveDrop();
	virtual ~CaveDrop();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

public:

private:

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	Animation* currentAnimation = nullptr;
	Animation startAnim;

	//L08 TODO 4: Add a physics to an item
	PhysBody* pbody;
};
