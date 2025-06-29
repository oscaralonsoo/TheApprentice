#pragma once

#include <memory>
#include <list>
#include "Module.h"
#include "Timer.h"
#include "PerfTimer.h"
#include "pugixml.hpp"
#include "Menus.h"

// Modules
class Window;
class Input;
class Render;
class Textures;
class Audio;
class Scene;
class EntityManager;
class ParticleManager;
class DialogueManager;
class PressureSystemController;
class Map;
class GameMap;
class Menus;
//L08 TODO 2: Add Physics module
class Physics;
class GuiManager;
class Engine
{
public:

	// Public method to get the instance of the Singleton
	static Engine& GetInstance();

	//	
	void AddModule(std::shared_ptr<Module> module);

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	bool PreUpdate();

	// Called each loop iteration
	bool Update();

	// Called before quitting
	bool CleanUp();

	bool UpdateConfig();

	float GetDt() const {
		return dt;
	}

private:

	// Private constructor to prevent instantiation
	// Constructor
	Engine();

	// Delete copy constructor and assignment operator to prevent copying
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	// Call modules before each loop iteration
	void PrepareUpdate();

	// Call modules before each loop iteration
	void FinishUpdate();

	// Call modules on each loop iteration
	bool DoUpdate();

	// Call modules after each loop iteration
	bool PostUpdate();

	// Load config file
	bool LoadConfig();



	std::list<std::shared_ptr<Module>> moduleList;

public:

	enum EngineState
	{
		CREATE = 1,
		AWAKE,
		START,
		LOOP,
		CLEAN,
		FAIL,
		EXIT
	};

	// Modules
	std::shared_ptr<Window> window;
	std::shared_ptr<Input> input;
	std::shared_ptr<Render> render;
	std::shared_ptr<Textures> textures;
	std::shared_ptr<Audio> audio;
	std::shared_ptr<Scene> scene;
	std::shared_ptr<EntityManager> entityManager;
	std::shared_ptr<ParticleManager> particleManager;
	std::shared_ptr<Map> map;
	std::shared_ptr<GameMap> gameMap;
	std::shared_ptr<DialogueManager> dialogueManager;
	std::shared_ptr<PressureSystemController> pressureSystem;
	std::shared_ptr<Menus> menus;
	std::shared_ptr<Physics> physics;
	std::shared_ptr<GuiManager> guiManager;

private: 

	// Delta time
	float dt; 
	//Frames since startup
	int frames;

	// Calculate timing measures
	// required variables are provided:
	Timer startupTime;
	PerfTimer frameTime;
	PerfTimer lastSecFrameTime;

	int frameCount = 0;
	int framesPerSecond = 0;
	int lastSecFrameCount = 0;

	float averageFps = 0.0f;
	int secondsSinceStartup = 0;

	//Maximun frame duration in miliseconds.
	int maxFrameDuration = 16;

	std::string gameTitle = "The Apprentice";

	//L05 TODO 2: Declare a xml_document to load the config file
	pugi::xml_document configFile;
};