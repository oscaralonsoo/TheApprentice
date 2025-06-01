#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Timer.h"
#include "IHookable.h"

class HookAnchor : public Entity, public IHookable
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
    bool IsPlayerWithinSensorRadius() const;
    void ResetHook();
    bool IsHookUsed() const { return hookUsed; }
    PhysBody* GetPhysBody() const { return pbody; }
    void Use();
    Vector2D GetRenderPosition() const override { return position; }
    int GetRenderWidth() const override { return width; }
    int GetRenderHeight() const override { return height; }

private:
    PhysBody* pbody = nullptr;
    PhysBody* sensor = nullptr;
    SDL_Texture* texture = nullptr;

    int width = 0;
    int height = 0;

    Timer hookTimer;
    bool isHooking = false;
    float hookDuration = 350.0f; // milisegundos
    bool cancelledByProximity = false;
    float cancelDistanceThreshold = 30.0f;
    bool hookUsed = false;
    bool wasOnGroundAtHookStart = false;
    SDL_Texture* highlightTexture = nullptr;

    bool transitionToHook = false;
};
