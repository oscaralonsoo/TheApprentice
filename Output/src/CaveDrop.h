#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Physics.h"

struct SDL_Texture;

enum class CaveDropStates
{
	START,
	FALL,
	SPLASH
};

class CaveDrop : public Entity
{
public:
	CaveDrop();
	virtual ~CaveDrop();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool CleanUp();

	void ChangeState(CaveDropStates newState);
	bool HasHitGround();
	void MarkForDeletion();

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

	CaveDropStates state = CaveDropStates::START;

	PhysBody* pbody;
};