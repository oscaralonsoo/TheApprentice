#pragma once

#include "Entity.h"
#include "Physics.h"
#include "Textures.h"
#include "Animation.h"
#include "Vector2D.h"

class DreadspireBullet : public Entity {
public:
    DreadspireBullet(float x, float y, float speed, b2Vec2 direction);
    ~DreadspireBullet();

    bool Update(float dt) override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

private:
    PhysBody* pbody = nullptr;
    SDL_Texture* texture = nullptr;
    Animation* currentAnimation = nullptr;

    Animation idleAnim;

    float speed;
    b2Vec2 direction;

    float time = 0.0f;
};
