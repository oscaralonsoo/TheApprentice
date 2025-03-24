#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"

struct SDL_Texture;

enum class SceneState 
{
	MAINMENU,
	NEWGAME,
	CONTINUE,
	PAUSE,
	SETTINGS,
	CREDITS,
	EXIT
};
class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Called each Update Iteration
	void UpdateTransition(float dt);

	// Called before starting the Transition
	void StartTransition(int nextScene);

	// Called before changing the scene
	void ChangeScene(int nextScene);

	// Return the player position
	Vector2D GetPlayerPosition();

public:

	Vector2D newPosition;

private:
	SDL_Texture* img;

	//L03: TODO 3b: Declare a Player attribute
	Player* player;

	//transition 
	bool transitioning = false;
	bool fadingIn = false;
	float transitionAlpha = 0.0f;
	int nextScene;
};