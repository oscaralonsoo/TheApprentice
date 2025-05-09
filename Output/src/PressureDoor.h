
#pragma once
#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Timer.h"

struct SDL_Texture;

enum class PressureDoorState {
    DISABLED, ENABLED
};

class PressureDoor : public Entity {
public:
    PressureDoor();
    virtual ~PressureDoor();

    bool Awake();

    bool Start();

    bool Update(float dt);

    bool PostUpdate();

    bool CleanUp();

    void RenderTexture();

    void SetOpen(bool isOpen);

public:
    int texW, texH;
    int height, width;
    int id;
    bool isOpen;
private:
    PhysBody* pbody;
    SDL_Texture* texture;

    PressureDoorState state = PressureDoorState::DISABLED;

    Animation disabledAnim;
    Animation enabledAnim;
    Animation* currentAnimation = nullptr;
};
