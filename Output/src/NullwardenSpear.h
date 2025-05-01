#pragma once

#include "Entity.h"
#include "Physics.h"
#include "Textures.h"
#include "Animation.h"
#include "Vector2D.h"

class NullwardenSpear : public Entity {
public:
    NullwardenSpear(float x, float y, bool horizontal, float speed, b2Vec2 direction);
    ~NullwardenSpear();

    bool Update(float dt) override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

private:
    PhysBody* pbody = nullptr;
    SDL_Texture* texture = nullptr;
    Animation* currentAnimation = nullptr;

    Animation spawnAnim;
    Animation idleAnim;
    Animation destroyedAnim;

    Vector2D position;
    int width, height;
    b2Vec2 direction;
    bool isHorizontal;
};
