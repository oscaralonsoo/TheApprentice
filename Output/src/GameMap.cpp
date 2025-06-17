#include "GameMap.h"
#include "Input.h"
#include "Engine.h"
#include "Textures.h"

GameMap::GameMap() : Module()
{
	name = "gameMap";
}

GameMap::~GameMap() {}

bool GameMap::Awake() {
	return true;
}

bool GameMap::Start() {
	gameMapTexture = Engine::GetInstance().textures->Load("Assets/GameMap/Map.png");

	return true;
}

bool GameMap::Update(float dt) {

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		showGameMap = !showGameMap;
	}

	return true;
}

bool GameMap::PostUpdate() {
	if (showGameMap) RenderGameMap();
	return true;
}

bool GameMap::CleanUp() {
	return true;
}

void GameMap::RenderGameMap() {
	int windowWidth, windowHeight;
	SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &windowWidth, &windowHeight);

	SDL_Rect camera = Engine::GetInstance().render->camera;

	Engine::GetInstance().render.get()->DrawTexture(gameMapTexture, (int)-camera.x, (int)-camera.y);
	
}