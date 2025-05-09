
#pragma once
#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "PressureSystemController.h"
#include "Timer.h"

struct SDL_Texture;

enum class PressurePlateState {
    DISABLED, ENABLED
};

class PressurePlate : public Entity {
public:
    PressurePlate();
    virtual ~PressurePlate();

    bool Awake();

    bool Start();

    bool Update(float dt);

    bool PostUpdate();

    bool CleanUp();

    void OnCollision(PhysBody* physA, PhysBody* physB);
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void SetActive(bool value);
    bool IsActive() const { return active; }

    void RenderTexture();

public:
    int texW, texH;
    int height, width;
    int id;
    bool active;

private:
    PhysBody* pbody;
    SDL_Texture* texture;

    PressurePlateState state = PressurePlateState::DISABLED;

    Animation disabledAnim;
    Animation enabledAnim;
    Animation* currentAnimation = nullptr;
};
