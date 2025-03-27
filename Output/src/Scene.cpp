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
#include "Item.h"
#include "Physics.h"
#include "Enemy.h"
#include "Menus.h"


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

	//L08 Create a new item using the entity manager and set the position to (200, 672) to test
	Item* item = (Item*) Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
	item->position = Vector2D(200, 672);
	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load("Assets/Maps/", "Map0.tmx");

	Engine::GetInstance().entityManager->CreateEnemiesFromXML(configParameters.child("save_data").child("enemies"),false);

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
	UpdateTransition(dt);
	Engine::GetInstance().render.get()->UpdateCamera(player->GetPosition(), player->GetMovementDirection(), 0.05);
	
	//L03 TODO 3: Make the camera movement independent of framerate
	float camSpeed = 1;

	if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
		Engine::GetInstance().render.get()->camera.y -= ceil(camSpeed * dt);

	if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
		Engine::GetInstance().render.get()->camera.y += ceil(camSpeed * dt);

	if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
		Engine::GetInstance().render.get()->camera.x -= ceil(camSpeed * dt);

	if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
		Engine::GetInstance().render.get()->camera.x += ceil(camSpeed * dt);

	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if (transitioning) {
		SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, static_cast<Uint8>(transitionAlpha * 255));
		SDL_RenderFillRect(Engine::GetInstance().render->renderer, nullptr);
	}

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	SDL_DestroyTexture(img);

	return true;
}
void Scene::StartTransition(int nextScene)
{
	if (!transitioning) {
		transitioning = true;
		fadingIn = false;
		transitionAlpha = 0.0f;
		this->nextScene = nextScene;
	}
}
// Called every iteration
void Scene::UpdateTransition(float dt)
{
	if (!transitioning) return;

	if (!fadingIn) { // Fade Out
		transitionAlpha += dt * 0.0025f;
		if (transitionAlpha >= 1.0f) {
			transitionAlpha = 1.0f;
			fadingIn = true;

			ChangeScene(nextScene);
		}
	}
	else { // Fade In
		transitionAlpha -= dt * 0.0020f;
		if (transitionAlpha <= 0.0f) {
			transitionAlpha = 0.0f;
			transitioning = false;
		}
	}
}

// Called before changing the scene
void Scene::ChangeScene(int nextScene)
{

	Engine::GetInstance().map->CleanUp(); 	// CleanUp of the previous Map

	Engine::GetInstance().entityManager.get()->DestroyAllEntities(); // Previous Enemies CleanUp

	// Look for the XML node
	std::string mapKey = "Map_" + std::to_string(nextScene);
	pugi::xml_node mapNode = configParameters.child("maps").child(mapKey.c_str());

	if (mapNode) {
		std::string path = mapNode.attribute("path").as_string();
		std::string name = mapNode.attribute("name").as_string();

		if (!path.empty() && !name.empty()) {
			Engine::GetInstance().map->Load(path, name); // Load New Map

			player->pbody->body->SetLinearVelocity(b2Vec2(0, 0)); // Stop All Movement
			player->pbody->body->SetTransform(b2Vec2(newPosition.x / PIXELS_PER_METER, newPosition.y / PIXELS_PER_METER), 0); // Set New Player Position

			Engine::GetInstance().entityManager->CreateEnemiesFromXML(configParameters.child("save_data").child("enemies"),true); // Create New Map Enemies
		}
	}
}

Vector2D Scene::GetPlayerPosition()
{
	return player->GetPosition();
}
