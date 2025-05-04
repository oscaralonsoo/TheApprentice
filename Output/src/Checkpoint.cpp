#include "Checkpoint.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

Checkpoint::Checkpoint() : Entity(EntityType::CHECKPOINT), state(CheckpointState::UNSAVED)
{
    name = "Checkpoint";
}

Checkpoint::~Checkpoint()
{
}

bool Checkpoint::Awake()
{
	return false;
}

bool Checkpoint::Start()
{
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    pugi::xml_node lifePlantNode = loadFile.child("config").child("scene").child("animations").child("props").child("life_plant");

    // Cargar textura y animaciones
    texture = Engine::GetInstance().textures->Load(lifePlantNode.attribute("texture").as_string());
    savingAnim.LoadAnimations(lifePlantNode.child("saving"));
    savedAnim.LoadAnimations(lifePlantNode.child("saved"));
    unsavedAnim.LoadAnimations(lifePlantNode.child("unsaved"));

    texW = lifePlantNode.attribute("w").as_int();
    texH = lifePlantNode.attribute("h").as_int();
    pbody = Engine::GetInstance().physics->CreateRectangleSensor(
        (int)position.getX() + texW / 2,
        (int)position.getY() + texH / 2,
        64, 64,
        STATIC,
        CATEGORY_SAVEGAME,      // Solo SaveGame
        CATEGORY_PLAYER          // Solo interactúa con el jugador
    );

    pbody->ctype = ColliderType::CHECKPOINT;
    pbody->listener = this;
    pbody->body->SetGravityScale(0);

    Engine::GetInstance().physics->listToDelete.push_back(pbody);

    currentAnimation = &unsavedAnim;
	return false;
}

bool Checkpoint::Update(float dt)
{
	return false;
}

bool Checkpoint::PostUpdate()
{
	return false;
}

bool Checkpoint::CleanUp()
{
	return false;
}
