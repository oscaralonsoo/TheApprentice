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
    pugi::xml_node checkpointNode = loadFile.child("config").child("scene").child("animations").child("props").child("checkpoint");

    // Cargar textura y animaciones
    texture = Engine::GetInstance().textures->Load(checkpointNode.attribute("texture").as_string());
    savingAnim.LoadAnimations(checkpointNode.child("saving"));
    savedAnim.LoadAnimations(checkpointNode.child("saved"));
    unsavedAnim.LoadAnimations(checkpointNode.child("unsaved"));

    // Obtener el tamaño del checkpoint
    texW = checkpointNode.attribute("w").as_int();
    texH = checkpointNode.attribute("h").as_int();

    // Centrar el sensor físico
    pbody = Engine::GetInstance().physics->CreateRectangleSensor(
        (int)position.getX() + width / 2,
        (int)position.getY() + height / 2,
        width, height,
        STATIC,
        CATEGORY_SAVEGAME,
        CATEGORY_PLAYER
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
        if (savingAnim.HasFinished()) {
            state = CheckpointState::SAVED;
            Engine::GetInstance().scene->SaveGameXML();
        }
        break;
    case CheckpointState::SAVED:
        if (currentAnimation != &savedAnim) currentAnimation = &savedAnim;
        break;
    }

    // Actualizar posición basada en el cuerpo físico
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - (texW/2)); 
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y)- (texH/2)); 


    return true;
}

bool Checkpoint::PostUpdate()
{
if (currentAnimation != nullptr) {
    const SDL_Rect& frame = currentAnimation->GetCurrentFrame();

    // Dibuja desde la base del collider hacia arriba
    int drawX = (int)position.getX();
    int drawY = (int)position.getY() + texH - frame.h + 5;

    Engine::GetInstance().render->DrawTexture(texture, drawX, drawY, &frame);
    currentAnimation->Update();
}
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
        if (state == CheckpointState::UNSAVED) {
            state = CheckpointState::SAVING;
        }
        else if (state == CheckpointState::SAVED) {
            Engine::GetInstance().scene->SaveGameXML();
        }

        // TODO JAVI --- PLAYER STOP MOVING
    }
}

