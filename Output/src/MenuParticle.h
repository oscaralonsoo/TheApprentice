#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Timer.h"

struct SDL_Texture;

enum class MenuParticleState {
    SPAWNING,
    FLOATING,
    FADING
};

class MenuParticle : public Entity
{
public:
    MenuParticle();
    ~MenuParticle();

    bool Awake();
    bool Start();
    bool Update();
    bool PostUpdate();
    bool CleanUp();

    void SetPosition(Vector2D pos);

private:
    SDL_Texture* texture = nullptr;
    int texW = 0, texH = 0;
    float scale = 1.0f;

    Vector2D velocity;

    Animation spawnAnim;
    Animation floatAnim;
    Animation fadeAnim;
    Animation* currentAnimation = nullptr;

    MenuParticleState state = MenuParticleState::SPAWNING;

    Timer floatTimer;
};
