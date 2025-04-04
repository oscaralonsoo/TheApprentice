#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Physics.h"
#include "Timer.h"

struct SDL_Texture;

const int MIN_RANDOM_TIME = 1;
const int MAX_RANDOM_TIME = 10;

enum class CaveDropStates
{
	START,
	FALL,
	SPLASH,
	DISABLED
};

class CaveDrop : public Entity
{
public:
	CaveDrop();
	~CaveDrop();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool CleanUp();

	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

private:
	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	Animation* currentAnimation = nullptr;
	Animation startAnim;
	Animation fallAnim;
	Animation splashAnim;

	Vector2D initPos;
	CaveDropStates state = CaveDropStates::START;

	int randomTime;
	Timer dropTimer;

	PhysBody* pbody;
};