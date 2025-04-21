#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Physics.h"

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
    PhysBody* pbody;
    SDL_Texture* texture;
    const char* texturePath;
    int texW, texH;
    float scale = 1.0f;

    Animation spawnAnim;
    Animation floatAnim;
    Animation fadeAnim;
    Animation* currentAnimation = nullptr;

    DustParticleState state = DustParticleState::SPAWNING;

    Timer floatTimer;
};
