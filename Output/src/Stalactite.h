#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Physics.h"
#include "Timer.h"

struct SDL_Texture;

enum class StalactiteState
{
    IDLE,
    TREMBLING,
    FALLING,
    SPLASHED
};

class Stalactite : public Entity
{
public:
    Stalactite();
    ~Stalactite();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate();
    bool CleanUp() override;

    void OnCollision(PhysBody* physA, PhysBody* physB) override;

private:
    SDL_Texture* texture;
    int texW, texH;

    Animation* currentAnimation = nullptr;
    Animation idleAnim;
    Animation fallAnim;
    Animation splashAnim;

    StalactiteState state = StalactiteState::IDLE;

    PhysBody* pbody;
    SDL_Point playerPoint;
    SDL_Rect triggerZone;
    bool playerUnder = false;
};

