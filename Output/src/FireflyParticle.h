#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Timer.h"
#include "Vector2D.h"

struct SDL_Texture;

enum class FireflyParticleState {
    SPAWNING,
    FLOATING,
    FADING
};

class FireflyParticle : public Entity
{
public:
    FireflyParticle();
    ~FireflyParticle();

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

    FireflyParticleState state = FireflyParticleState::SPAWNING;

    Timer floatTimer;

    // Nuevos atributos para movimiento tipo luciérnaga
    float directionAngle = 0.0f;         // Dirección actual en radianes
    float angleChangeCooldown = 0.0f;   // Tiempo restante antes de cambiar dirección
};
