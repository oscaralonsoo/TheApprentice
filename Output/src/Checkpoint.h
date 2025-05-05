
#pragma once
#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Physics.h"
#include "Timer.h"
#include "PlayerMechanics.h"

struct SDL_Texture;

enum class CheckpointState {
    UNSAVED, SAVING, SAVED 
};

class Checkpoint : public Entity {
public:
    Checkpoint();
    virtual ~Checkpoint();

    bool Awake();

    bool Start();

    bool Update(float dt);

    bool PostUpdate();

    bool CleanUp();

    void OnCollision(PhysBody* physA, PhysBody* physB);

    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void CheckSave();

public:
    int texW, texH;
    int height, width;

private:
    PhysBody* pbody;
    SDL_Texture* texture;

    CheckpointState state = CheckpointState::UNSAVED;

    Animation savingAnim;
    Animation savedAnim;
    Animation unsavedAnim;
    Animation* currentAnimation = nullptr;

    const char* texturePath;


    bool isSaving = false;
    bool insideCheckpoint = false;
};
