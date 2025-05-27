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

	virtual void OnCollision(PhysBody* physA, PhysBody* physB);

	virtual void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	int GetWidth() const { return width; }

	int GetHeight() const { return height; }

	void SetWidth(int width);

	void SetHeight(int height);

private:
	PhysBody* pbody;

	int alpha = 255;
	bool fadingIn = false;
	bool fadingOut = false;
	float fadeSpeed = 1.0f;

	bool alreadyRevealed = false;
};
