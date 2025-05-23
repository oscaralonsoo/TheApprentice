#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Timer.h"

struct SDL_Texture;

enum class DustParticleState {
    SPAWNING,
    FLOATING,
    FADING
};

class DustParticle : public Entity
{
public:
    DustParticle();
    ~DustParticle();

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

    Animation spawnAnim;
    Animation floatAnim;
    Animation fadeAnim;
    Animation* currentAnimation = nullptr;

    DustParticleState state = DustParticleState::SPAWNING;

    Timer floatTimer;
};
