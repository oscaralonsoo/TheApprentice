#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include "PlayerMechanics.h"
#include "Checkpoint.h"
#include "HookAnchor.h"
#include "HookManager.h"

struct SDL_Texture;

struct Checkpoint;

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

	void Vignette(int size, float strength, SDL_Color color);

	void VignetteChanges(float dt);

	void TriggerVignetteFlash();
	Player* GetPlayer() const { return player; }

	void SetActiveHook(HookAnchor* hook);

	HookAnchor* GetActiveHook() const;

	HookManager* GetHookManager() const { return hookManager; }


public:
	bool isChangingScene = false;
	bool isLoading = false;
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
	SDL_Color vignetteColor;
	bool pendingLoadAfterDeath = false;

	PlayerMechanics* mechanics = nullptr;

	SDL_Texture* slimeLoading = nullptr;

	float heartbeatTimer = 0.0f;
	float heartbeatInterval = 2000.0f;
	bool heartbeatGrowing = false;
	float heartbeatProgress = 0.0f; 

	// Vignette flash effect
	bool vignetteFlashActive = false;
	float vignetteFlashTimer = 0.0f;
	const float vignetteFlashDuration = 1000.0f; 
	SDL_Color originalVignetteColor;
	float vignetteLerpProgress = 0.0f;
	SDL_Color vignetteTargetColor;


	float blackScreenTimer = 0.0f;
	const float blackScreenDelay = 500.0f;
	bool waitingBlackScreen = false;

	int soundUI1Id = 0;
	int soundUI2Id = 0;
	bool ui1SoundPlayed = false;
	bool ui2SoundPlayed = false;

private:
	SDL_Texture* img;
	//L03: TODO 3b: Declare a Player attribute
	Player* player;
	Checkpoint* checkpoint = nullptr;

	//transition 
	bool fadingIn = false;
	float transitionAlpha = 0.0f;
	bool pendingLoadWithTransition = false;

	//Vignette
	float distFactor = 0.0f;
	float opacity = 0.0f;
	int width, height;
	Uint8 alpha;
	SDL_Rect top, bottom, left, right;



	//Renderer
	SDL_Renderer* renderer;

	HookAnchor* activeHook = nullptr;

	HookManager* hookManager = nullptr;
};