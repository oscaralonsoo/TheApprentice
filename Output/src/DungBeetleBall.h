#pragma once

#include "Entity.h"
#include "Physics.h"
#include "Textures.h"
#include "Animation.h"
#include "Vector2D.h"

class DungBeetleBall : public Entity {
public:
    DungBeetleBall(float x, float y, float speed, b2Vec2 direction);
    ~DungBeetleBall();

    bool Update(float dt) override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB);

    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void Bounce();

private:
    PhysBody* pbody = nullptr;
    SDL_Texture* texture = nullptr;
    Animation* currentAnimation = nullptr;

    Animation idleAnim;
    float timeStuck = 0.0f;
    b2Vec2 previousPosition;
    Vector2D position;
    int width;
    int height;
    float speed;
    b2Vec2 direction;

    float time = 0.0f;
};
