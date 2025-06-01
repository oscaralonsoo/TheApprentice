#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Physics.h"
#include "Timer.h"

class PushableBox : public Entity
{
public:
    PushableBox();
    virtual ~PushableBox();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;

    void SetPosition(Vector2D pos);
    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;

    Vector2D GetPosition() const;

    void SetParameters(pugi::xml_node parameters) {
        this->parameters = parameters;
        position.x = parameters.attribute("x").as_int();
        position.y = parameters.attribute("y").as_int();
        width = parameters.attribute("w").as_int();
        height = parameters.attribute("h").as_int();
    }

protected:
    PhysBody* pbody = nullptr;
    SDL_Texture* texture = nullptr;
    pugi::xml_node parameters;

    bool isPlayerPushing = false;
    bool isEnemyPushing = false;

    bool transitionToPush = false;
};
