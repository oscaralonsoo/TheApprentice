#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Timer.h"

class HookAnchor : public Entity
{
public:
    HookAnchor();
    virtual ~HookAnchor();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;

    void SetParameters(pugi::xml_node parameters);
    void SetPosition(Vector2D pos);
    Vector2D GetPosition() const;

    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;
    void EndHook();

private:
    PhysBody* pbody = nullptr;
    PhysBody* sensor = nullptr;
    SDL_Texture* texture = nullptr;

    int width = 0;
    int height = 0;

    bool playerInRange = false;

    Timer hookTimer;
    bool isHooking = false;
    float hookDuration = 300.0f; // milisegundos
    bool cancelledByProximity = false;
    float cancelDistanceThreshold = 30.0f;
    bool hookUsed = false;
};
