#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"

class DestructibleWall : public Entity
{
public:
    DestructibleWall();
    virtual ~DestructibleWall();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;

    void SetParameters(pugi::xml_node parameters);
    void SetPosition(Vector2D pos);
    Vector2D GetPosition() const;

    void OnCollision(PhysBody* physA, PhysBody* physB) override;

private:
    PhysBody* pbody = nullptr;
    SDL_Texture* texture = nullptr;
    int texW = 0;
    int texH = 0;
    bool destroyed = false;

};
