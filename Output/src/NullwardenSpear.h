#pragma once

#include "Entity.h"
#include "Physics.h"
#include "Textures.h"
#include "Animation.h"
#include "Vector2D.h"

class NullwardenSpear : public Entity {
public:
    NullwardenSpear(float x, float y, float speed, b2Vec2 direction);
    ~NullwardenSpear();

    bool Update(float dt) override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

    void Activate();
    void Deactivate();

private:
    PhysBody* pbody = nullptr;
    SDL_Texture* texture = nullptr;
    Animation* currentAnimation = nullptr;

    Animation spawnAnim;
    Animation idleAnim;
    Animation destroyedAnim;

    b2Vec2 direction;
};
