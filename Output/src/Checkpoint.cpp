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
        (int)position.getX(),
        (int)position.getY(),
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
	return true;
}

bool Checkpoint::Update(float dt)
{
    CheckSave();
    switch (state) {
    case CheckpointState::UNSAVED:
        if (currentAnimation != &unsavedAnim) currentAnimation = &unsavedAnim;

        break;
    case CheckpointState::SAVING:
        if (currentAnimation != &savingAnim) currentAnimation = &savingAnim;
        //cuando acabe la anim, que se cambie a SAVED
        break;
    case CheckpointState::SAVED:
        if (currentAnimation != &savedAnim) currentAnimation = &savedAnim;
        break;
    }
    // Actualizar posición basada en el cuerpo físico
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    if (currentAnimation != nullptr) {
        Engine::GetInstance().render->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
        currentAnimation->Update();
    }
	return true;
}

bool Checkpoint::PostUpdate()
{
	return true;
}

bool Checkpoint::CleanUp()
{
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void Checkpoint::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        insideCheckpoint = true;
        
        break;
    }
}
void Checkpoint::OnCollisionEnd(PhysBody* physA, PhysBody* physB){
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        insideCheckpoint = false;
        break;
    }
}
void Checkpoint::CheckSave() {
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT && insideCheckpoint) {
        Engine::GetInstance().scene->SaveGameXML();
        state = CheckpointState::SAVING;
        // TODO JAVI --- PLAYER STOP MOVING
    }
}
