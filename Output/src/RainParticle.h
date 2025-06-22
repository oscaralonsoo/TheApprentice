#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Timer.h"

struct SDL_Texture;

class RainParticle : public Entity
{
public:
    RainParticle();
    ~RainParticle();

    bool Awake();
    bool Start();
    bool Update(float dt);
    bool PostUpdate();
    bool CleanUp();

    void SetPosition(Vector2D pos);

private:
    SDL_Texture* texture = nullptr;
    int texW = 0, texH = 0;
    float scale = 1.0f;
    Vector2D velocity;

    Animation* currentAnimation = nullptr;

    Animation idleAnim;

};
