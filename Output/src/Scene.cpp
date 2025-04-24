#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"

Scene::Scene() : Module()
{
	name = "scene";
	img = nullptr;
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	//L04: TODO 3b: Instantiate the player using the entity manager
	player = (Player*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER);
	player->SetParameters(configParameters.child("animations").child("player"));

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load("Assets/Maps/", "MapTemplate.tmx");

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	Engine::GetInstance().render.get()->camera.x = -(player->position.getX()-300);
	Engine::GetInstance().render.get()->camera.y = -(player->position.getY()-500);

	//L03 TODO 3: Make the camera movement independent of framerate
	float camSpeed = 1;

	//if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
	//	Engine::GetInstance().render.get()->camera.y -= ceil(camSpeed * dt);

	//if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
	//	Engine::GetInstance().render.get()->camera.y += ceil(camSpeed * dt);

	//if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
	//	Engine::GetInstance().render.get()->camera.x -= ceil(camSpeed * dt);

	//if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
	//	Engine::GetInstance().render.get()->camera.x += ceil(camSpeed * dt);
	//if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F7) == KEY_DOWN)
	//	ChangeScene(nextScene + 1);
	//if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F8) == KEY_DOWN)
	//	if (nextScene == 0)
	//	{
	//		ChangeScene(nextScene);
	//		player->SetPosition(Vector2D(1950, 650));
	//	}
	//	else {
	//		ChangeScene(nextScene - 1);
	//	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
		SaveGameXML();
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F10) == KEY_DOWN)
		LoadGameXML();

	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	SDL_DestroyTexture(img);

	return true;
}
