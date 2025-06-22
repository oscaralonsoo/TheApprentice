#include "ParticleManager.h"
#include "DustParticle.h"
#include "Engine.h"
#include "Textures.h"
#include "Scene.h"
#include "Window.h"
#include "FireflyParticle.h"
#include "RainParticle.h"
#include "SnowParticle.h"
#include "Render.h"
#include "Log.h"
#include "Scene.h"
#include <cstdlib>

ParticleManager::ParticleManager() : Module()
{
	name = "ParticleManager";
}

// Destructor
ParticleManager::~ParticleManager()
{}

// Called before render is available
bool ParticleManager::Awake()
{
	LOG("Loading Particle Manager");
	bool ret = true;

	//Iterates over the entities and calls the Awake
	for(const auto particle : particles)
	{
		if (particle->active == false) continue;
		ret = particle->Awake();
	}

	return ret;

}

bool ParticleManager::Start() {

	bool ret = true; 

	//Iterates over the entities and calls Start
	for(const auto particle : particles)
	{
		if (particle->active == false) continue;
		ret = particle->Start();
	}

	return ret;
}

// Called before quitting
bool ParticleManager::CleanUp()
{
	bool ret = true;

	for(const auto particle : particles)
	{
		if (particle->active == false) continue;
		ret = particle->CleanUp();
	}

	particles.clear();

	return ret;
}

// Function to destroy a specific Particle
void ParticleManager::DestroyParticle(Entity* particle)
{
	for (auto it = particles.begin(); it != particles.end();)
	{
		if (*it == particle) {
			(*it)->CleanUp();
			delete* it;
			it = particles.erase(it); // devuelve el siguiente iterador
			break;
		}
		else {
			++it;
		}
	}
}

// Function to destroy All Entities & Not The Player
void ParticleManager::DestroyAllParticles() {

	for (auto it = particles.begin(); it != particles.end(); ) {
		LOG("Destroying particle at position: (%f, %f)", (*it)->position.getX(), (*it)->position.getY());
		(*it)->CleanUp();
		delete* it;
		it = particles.erase(it);
		
	}
	LOG("All particles removed.");
}

bool ParticleManager::Update(float dt)
{
	if (Engine::GetInstance().menus->currentState != MenusState::GAME || Engine::GetInstance().menus->isPaused)
		return true;

	int currentScene = Engine::GetInstance().scene->nextScene;

	if (currentScene != lastScene)
	{
		DestroyAllParticles();
		lastScene = currentScene;
	}

	SetParticlesByMap(currentScene);

	for (auto particle : particles)
	{
		if (particle->active)
			particle->Update(dt);
	}

	return true;
}



bool ParticleManager::PostUpdate()
{
	bool ret = true;

	// Copia segura de las entidades activas
	std::vector<Entity*> activeParticles;
	for (auto particle : particles)
	{
		if (particle->active)
			activeParticles.push_back(particle);
	}

	// Ahora iteramos sobre la copia, aunque la original se modifique
	for (auto particle : activeParticles)
	{
		ret = particle->PostUpdate();
	}

	return ret;
}

void ParticleManager::SpawnDustParticles(DustParticleVariant variant)
{
	if (rand() % 100 < 12)
	{
		int windowWidth, windowHeight;
		SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &windowWidth, &windowHeight);

		SDL_Rect camera = Engine::GetInstance().render->camera;

		int randX = rand() % windowWidth;
		int randY = rand() % windowHeight;

		Vector2D posMap = Engine::GetInstance().map.get()->WorldToMap(randX - camera.x, randY - camera.y);

		MapLayer* layer = Engine::GetInstance().map.get()->GetNavigationLayer();

		if (!layer->Get(posMap.x, posMap.y)) {
			DustParticle* particle = new DustParticle((int)variant);
			particles.push_back(particle);
			particle->Start();
			particle->SetPosition({ (float)randX - camera.x, (float)randY - camera.y });
		}
	}
}

void ParticleManager::SpawnFireflyParticles()
{
	if (rand() % 100 < 5)
	{
		int windowWidth, windowHeight;
		SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &windowWidth, &windowHeight);

		SDL_Rect camera = Engine::GetInstance().render->camera;

		int randX = rand() % windowWidth;
		int randY = rand() % windowHeight;

		Vector2D posMap = Engine::GetInstance().map.get()->WorldToMap(randX - camera.x, randY - camera.y);

		MapLayer* layer = Engine::GetInstance().map.get()->GetNavigationLayer();

		if (!layer->Get(posMap.x, posMap.y)) {
			FireflyParticle* particle = new FireflyParticle();
			particles.push_back(particle);
			particle->Start();
			particle->SetPosition({ (float)randX - camera.x, (float)randY - camera.y });
		}
	}
}

void ParticleManager::SpawnRainParticles() {
	const int xOffset = 500;

	if (rand() % 100 < 25)
	{
		Vector2D camPos = Vector2D(
			Engine::GetInstance().render->camera.x * -1,
			Engine::GetInstance().render->camera.y * -1
		);

		int windowWidth, windowHeight;
		SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &windowWidth, &windowHeight);

		RainParticle* particle = new RainParticle();
		particles.push_back(particle);
		particle->Start();

		float spawnX = camPos.x - xOffset + static_cast<float>(rand() % (windowWidth + 2 * xOffset));
		float spawnY = camPos.y - 20.0f;

		particle->SetPosition({ spawnX, spawnY });
	}
}

void ParticleManager::SpawnSnowParticles() {
	const int xOffset = 500;

	if (rand() % 100 < 80)
	{
		Vector2D camPos = Vector2D(
			Engine::GetInstance().render->camera.x * -1,
			Engine::GetInstance().render->camera.y * -1
		);

		int windowWidth, windowHeight;
		SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &windowWidth, &windowHeight);

		SnowParticle* particle = new SnowParticle();
		particles.push_back(particle);
		particle->Start();

		float spawnX = camPos.x - xOffset + static_cast<float>(rand() % (windowWidth + 2 * xOffset));
		float spawnY = camPos.y - 20.0f;

		particle->SetPosition({ spawnX, spawnY });
	}
}



void ParticleManager::SetParticlesByMap(int scene) {

	switch (scene) {
	case 0:
	case 666:
	case 6666:
	case 99:
		SpawnDustParticles(DustParticleVariant::CRYSTAL_CAVE);
		break;
	case 1:
	case 12:
	case 13:
	case 14:
		SpawnDustParticles(DustParticleVariant::CAVE);
		break;
	case 21:
	case 22:
	case 23:
		SpawnRainParticles();
		SpawnFireflyParticles();
		break;
	case 69:
		SpawnFireflyParticles();
		break;
	case 31:
	case 41:
	case 42:
	case 43:
		SpawnSnowParticles();
		break;
	default:
		SpawnDustParticles(DustParticleVariant::CAVE);
		break;
	}
}