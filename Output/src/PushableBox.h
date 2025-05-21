#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Physics.h"

class PushableBox : public Entity
{
public:
    PushableBox();
    virtual ~PushableBox();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;

    void SetParameters(pugi::xml_node parameters);
    void SetPosition(Vector2D pos);
    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;
    void RecreateBody(bodyType type);

    Vector2D GetPosition() const;

protected:
    PhysBody* pbody = nullptr;
    SDL_Texture* texture = nullptr;
    int texW = 0;
    int texH = 0;
    Vector2D position;
    bool touchingPlayer = false;
    bool isStatic = false;
};
