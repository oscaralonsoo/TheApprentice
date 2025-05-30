#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Timer.h"

struct SDL_Texture;

class SnowParticle : public Entity
{
public:
    SnowParticle();
    ~SnowParticle();

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

    float time = 0.0f;
    float baseX = 0.0f;
    float oscillationAmplitude = 2.0f;
    float oscillationFrequency = 1.0f;
    float phaseOffset = 0.0f;


};
