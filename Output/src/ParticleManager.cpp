#include "ParticleManager.h"
#include "DustParticle.h"
#include "Engine.h"
#include "Textures.h"
#include "Scene.h"
#include "Window.h"
#include "Render.h"
#include "Log.h"
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

Entity* ParticleManager::CreateParticle(EntityType type)
{
	Entity* particle = nullptr; 

	//L04: TODO 3a: Instantiate particle according to the type and add the new particle to the list of Entities
	switch (type)
	{
	case EntityType::DUST_PARTICLE:
		particle = new DustParticle();
		break;
	default:
		break;
	}

	particles.push_back(particle);

	return particle;
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

	if (rand() % 100 < 12) // % de probabilidad cada frame
	{
		SpawnRandomParticles();
	}

	// Actualizar partículas activas
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

void ParticleManager::SpawnRandomParticles()
{
	int windowWidth, windowHeight;
	SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &windowWidth, &windowHeight);

	SDL_Rect camera = Engine::GetInstance().render->camera;

	int randX = rand() % windowWidth;
	int randY = rand() % windowHeight;

	Vector2D posMap = Engine::GetInstance().map.get()->WorldToMap(randX - camera.x, randY - camera.y);

	MapLayer* layer = Engine::GetInstance().map.get()->GetNavigationLayer();

	if (!layer->Get(posMap.x, posMap.y)) {
		DustParticle* particle = (DustParticle*)CreateParticle(EntityType::DUST_PARTICLE);
		particle->Start();
		particle->SetPosition({ (float)randX - camera.x, (float)randY - camera.y });
	}
}