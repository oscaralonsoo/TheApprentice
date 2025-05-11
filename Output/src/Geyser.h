
#pragma once
#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Physics.h"
#include "Timer.h"

struct SDL_Texture;

enum class GeyserState {
    DISABLED, ENABLED
};

class Geyser : public Entity {
public:
    Geyser();
    virtual ~Geyser();

    bool Awake();

    bool Start();

    bool Update(float dt);

    bool PostUpdate();

    bool CleanUp();

    void OnCollision(PhysBody* physA, PhysBody* physB);
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void RenderTexture();

public:
    int texW, texH;
    int height, width;

private:
    PhysBody* pbody;
    SDL_Texture* texture;

    GeyserState state = GeyserState::DISABLED;

    Animation disabledAnim;
    Animation enabledAnim;
    Animation* currentAnimation = nullptr;

    Timer geyserTimer;
    float geyserCooldown = 3000;

    bool playerInside = false;
    bool hasPushed = false;
};
