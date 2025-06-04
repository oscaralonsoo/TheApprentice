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
    bool PostUpdate();
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB);

    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void Bounce();

    void CollisionNavigationLayer();

public:
    Animation* currentAnimation = nullptr;
    Animation destroyAnim;
    bool destroyBall = false;
private:
    SDL_Texture* texture = nullptr;
    PhysBody* pbody = nullptr;
    Animation idleAnim;
    float timeStuck = 0.0f;
    b2Vec2 previousPosition;
    float speed;
    b2Vec2 direction;

    float time = 0.0f;

    Vector2D currentTileMap;
};
