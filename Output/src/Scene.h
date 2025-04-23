#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"

struct SDL_Texture;

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

	Vector2D GetPlayerPosition();
	// Called When Saving Game
	void SaveGameXML();
	// Called When Loading Game
	void LoadGameXML();

	void Vignette(int size, float strength);

	Player* GetPlayer() const { return player; }

public:
	int previousVignetteSize;
	Vector2D newPosition;
	bool transitioning = false;
	bool saveGameZone = false;
	bool saving = false;
	int nextScene = 0;

	//Vignette
	int vignetteSize = 300;
	float vignetteStrength = 0.8f;

	bool isDead = false;

private:
	SDL_Texture* img;
	//L03: TODO 3b: Declare a Player attribute
	Player* player;
	bool isLoad = false;

	//transition 
	bool fadingIn = false;
	float transitionAlpha = 0.0f;

	//Vignette
	float distFactor = 0.0f;
	float opacity = 0.0f;
	int width, height;
	Uint8 alpha;
	SDL_Rect top, bottom, left, right;

	bool pendingLoadAfterDeath = false;

	//Renderer
	SDL_Renderer* renderer;
};