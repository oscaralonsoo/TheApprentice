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
#include "CaveDrop.h"
#include "Physics.h"
#include "Enemy.h"
#include "Menus.h"
#include "PlayerMechanics.h"
#include "Checkpoint.h"

template <typename T>
T Clamp(T value, T min, T max)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}
Scene::Scene() : Module()
{
	name = "scene";
	img = nullptr;
	vignetteColor = { 0, 0, 0, 255 };
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
	mechanics = player->GetMechanics();

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load("Assets/Maps/", "Map0.tmx");
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
	if (Engine::GetInstance().menus->currentState != MenusState::GAME)
		return true;

	if (pendingLoadAfterDeath) {
		pendingLoadAfterDeath = false;
		LoadGameXML();
	}

	UpdateTransition(dt);

	Engine::GetInstance().render.get()->UpdateCamera(player->GetPosition(), player->GetMovementDirection(), 0.05);
	
	//L03 TODO 3: Make the camera movement independent of framerate
	float camSpeed = 1;

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F7) == KEY_DOWN)
		ChangeScene(nextScene + 1);
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F8) == KEY_DOWN)
		if (nextScene == 0)
		{
			ChangeScene(nextScene);
			player->SetPosition(Vector2D(1950, 650));
		}
		else {
			ChangeScene(nextScene - 1);
		}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
		SaveGameXML();
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F10) == KEY_DOWN)
		LoadGameXML();
	// L�gica para el efecto de latidos
	VignetteHeartBeat(dt);
	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if (transitioning) {
		SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);
		Uint8 alpha = 255;

		if (!fadingIn && transitionAlpha < 1.0f) {
			alpha = static_cast<Uint8>(transitionAlpha * 255);
		}
		else if (waitingBlackScreen) {
			alpha = 255; // Pantalla negra completa
		}
		else if (fadingIn) {
			alpha = static_cast<Uint8>(transitionAlpha * 255);
		}

		SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, alpha);
		SDL_RenderFillRect(Engine::GetInstance().render->renderer, nullptr);
	}

	Vignette(player->GetMechanics()->GetHealthSystem()->GetVignetteSize(), 0.8f, vignetteColor);

	if (isDead) {
		isDead = false;
		pendingLoadAfterDeath = true;
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

	if (!fadingIn) { // FADE OUT
		transitionAlpha += dt * 0.0025f;
		if (transitionAlpha >= 1.0f) {
			transitionAlpha = 1.0f;

			if (!waitingBlackScreen) {
				ChangeScene(nextScene);          
				waitingBlackScreen = true;       
				blackScreenTimer = 0.0f;  
			}
			else {
				blackScreenTimer += dt;

				if (blackScreenTimer >= blackScreenDelay) {
					fadingIn = true;
					waitingBlackScreen = false;
				}
			}
		}
	}
	else { // FADE IN
		transitionAlpha -= dt * 0.0020f;
		if (transitionAlpha <= 0.0f) {
			transitionAlpha = 0.0f;
			transitioning = false;
			pendingLoadWithTransition = false;
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

			if (!isLoading)
			{
				player->pbody->body->SetLinearVelocity(b2Vec2(0, 0)); // Stop All Movement
				player->pbody->body->SetTransform(b2Vec2(newPosition.x / PIXELS_PER_METER, newPosition.y / PIXELS_PER_METER), 0); // Set New Player Position
			}

			Engine::GetInstance().entityManager->Start();
		}
	}
}

Vector2D Scene::GetPlayerPosition()
{
	return player->GetPosition();
}

void Scene::SaveGameXML() {
	saving = true;
	Engine::GetInstance().menus->isSaved = 1;

	// Load xml
	pugi::xml_document config;
	pugi::xml_parse_result result = config.load_file("config.xml");
	pugi::xml_node saveData = config.child("config").child("scene").child("save_data");

	Vector2D playerPos = GetPlayerPosition(); // Save Player Pos
	pugi::xml_node playerNode = saveData.child("player");
		playerNode.attribute("x") = playerPos.x;
		playerNode.attribute("y") = playerPos.y;
		playerNode.attribute("lives") = player->GetMechanics()->GetHealthSystem()->GetLives();

	pugi::xml_node abilitiesNode = saveData.child("abilities");
	abilitiesNode.attribute("jump") = player->GetMechanics()->GetMovementHandler()->IsJumpUnlocked();
	abilitiesNode.attribute("doublejump") = player->GetMechanics()->GetMovementHandler()->IsDoubleJumpUnlocked();
	abilitiesNode.attribute("dash") = player->GetMechanics()->GetMovementHandler()->IsDashUnlocked();

	pugi::xml_node sceneNode = saveData.child("scene"); // Save Actual Scene
	sceneNode.attribute("actualScene") = nextScene;
	saveData.attribute("isSaved") = Engine::GetInstance().menus->isSaved;
	config.save_file("config.xml"); // Save Changes

	Engine::GetInstance().menus->StartTransition(false, Engine::GetInstance().menus->currentState); // Final Transition

	mechanics->healthSystem.HealFull();//Heal The Player
}
void Scene::LoadGameXML() {
	if (isLoading || transitioning) return;

    isLoading = true;

    pugi::xml_document config;
    pugi::xml_parse_result result = config.load_file("config.xml");

    pugi::xml_node saveData = config.child("config").child("scene").child("save_data");

    if (saveData) {
        pugi::xml_node playerNode = saveData.child("player");
        if (playerNode) {
            float playerX = playerNode.attribute("x").as_float();
            float playerY = playerNode.attribute("y").as_float();
			player->GetMechanics()->GetHealthSystem()->SetLives(playerNode.attribute("lives").as_int());
            newPosition = Vector2D(playerX, playerY - 100); 
        }
		pugi::xml_node abilitiesNode = saveData.child("abilities"); // Abilities Load
		if (abilitiesNode) {
			if (abilitiesNode.attribute("jump").as_bool() == true) {
				mechanics->EnableJump(true);
			}
			if (abilitiesNode.attribute("doublejump").as_bool() == true) {
				mechanics->EnableDoubleJump(true);
			}
			if (abilitiesNode.attribute("dash").as_bool() == true) {
				mechanics->EnableDash(true);
			}
		}
        pugi::xml_node sceneNode = saveData.child("scene");
		if (sceneNode) {
			int savedScene = sceneNode.attribute("actualScene").as_int();
			nextScene = savedScene;
			if (!pendingLoadWithTransition) {
				pendingLoadWithTransition = true;
				StartTransition(savedScene);
			}
		}
	}

}
void Scene::Vignette(int size, float strength, SDL_Color color)
{
	renderer = Engine::GetInstance().render->renderer;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_GetRendererOutputSize(renderer, &width, &height);

	vignetteSize = size;

	if (heartbeatProgress > 0.0f) {
		vignetteSize += static_cast<int>(heartbeatProgress * 250); 
	}

	for (int i = 0; i < vignetteSize; i++)
	{
		distFactor = (float)i / vignetteSize;
		opacity = powf(1.0f - distFactor, 2) * strength;
		alpha = static_cast<Uint8>(opacity * 255);

		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);

		top = { 0, i, width, 1 };
		bottom = { 0, height - i - 1, width, 1 };
		left = { i, 0, 1, height };
		right = { width - i - 1, 0, 1, height };

		SDL_RenderFillRect(renderer, &top);
		SDL_RenderFillRect(renderer, &bottom);
		SDL_RenderFillRect(renderer, &left);
		SDL_RenderFillRect(renderer, &right);
	}
}
void Scene::VignetteHeartBeat(float dt)
{
	if (mechanics->GetHealthSystem()->GetLives() != 1)
	{
		heartbeatProgress = 0.0f;
		return;
	}
	heartbeatTimer += dt;
	if (heartbeatTimer >= heartbeatInterval)
	{
		heartbeatTimer = 0.0f;
		heartbeatGrowing = true;
	}
	float speed = heartbeatGrowing ? 0.01f : -0.005f;
	heartbeatProgress = Clamp(heartbeatProgress + dt * speed, 0.0f, 1.0f);
	if (heartbeatProgress == 1.0f)
		heartbeatGrowing = false;
}


