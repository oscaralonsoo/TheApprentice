#include "Checkpoint.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "PlayerMechanics.h"
#include "Player.h"
#include "MovementHandler.h"

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

    // Obtener el tama�o del checkpoint
    texW = checkpointNode.attribute("w").as_int();
    texH = checkpointNode.attribute("h").as_int();

    // Centrar el sensor f�sico
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
    growId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Props/checkpoint_grow.ogg", 1.0f);
    soundInteractId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Props/checkpoint_activation.ogg", 1.0f);

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

        if (!interactSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(growId, 0.4f, 0);
            Engine::GetInstance().audio->PlayFx(soundInteractId, 0.2f, 0);
            interactSoundPlayed = true;
        }

        if (currentAnimation != &savingAnim)  {
            currentAnimation = &savingAnim;
            Player* player = Engine::GetInstance().scene->GetPlayer();
            player->GetMechanics()->GetMovementHandler()->SetCantMove(false);
            Engine::GetInstance().scene->SaveGameXML();
        }
        if (savingAnim.HasFinished()) {
            state = CheckpointState::SAVED;
        }
        break;
    case CheckpointState::SAVED:
        if (currentAnimation != &savedAnim) currentAnimation = &savedAnim;
        Player* player = Engine::GetInstance().scene->GetPlayer();
        player->GetMechanics()->GetMovementHandler()->SetCantMove(false);
        break;
    }

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - (texW/2)); 
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y)- (texH/2)); 

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
    bool saveRequested = false;

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT) {
        saveRequested = true;
    }

    Player* player = Engine::GetInstance().scene->GetPlayer();
    SDL_GameController* controller = player->GetMechanics()->GetMovementHandler()->GetController();

    if (controller && SDL_GameControllerGetAttached(controller)) {
        bool yNow = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y);
        if (yNow && !yHeld) {
            saveRequested = true;
        }
        yHeld = yNow;
    }

    if (saveRequested && insideCheckpoint) {
        interactSoundPlayed = false;
        if (state == CheckpointState::UNSAVED) {
            state = CheckpointState::SAVING;
            player->GetMechanics()->GetMovementHandler()->SetCantMove(true);
        }
        else if (state == CheckpointState::SAVED) {
            Engine::GetInstance().audio->PlayFx(soundInteractId, 0.2f, 0);
            Engine::GetInstance().scene->SaveGameXML();

        }
        b2Vec2 stopVelocity(0.0f, 0.0f);
        player->pbody->body->SetLinearVelocity(stopVelocity);
    }
}

