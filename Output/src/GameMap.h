#pragma once

#include "Module.h"

struct SDL_Texture;

class GameMap : public Module
{
public:
	GameMap();
	virtual ~GameMap();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool PostUpdate();
	bool CleanUp();

	void RenderGameMap();

private:
	SDL_Texture* gameMapTexture;

	bool showGameMap = false;
};