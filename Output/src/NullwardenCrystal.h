#pragma once
#include "Entity.h"
#include "Physics.h"
#include "Textures.h"
#include "Animation.h"
#include "Vector2D.h"
#include "Nullwarden.h"

class Nullwarden;

class NullwardenCrystal : public Entity {
public:
    NullwardenCrystal(float x, float y, float speed, b2Vec2 dir, Nullwarden* owner);
    ~NullwardenCrystal();

    bool Update(float dt) override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;

public:
    int hits = 0;
private:
    Nullwarden* nullwarden = nullptr;
    PhysBody* pbody = nullptr;
    SDL_Texture* texture = nullptr;
    Animation* currentAnimation = nullptr;



    Animation pristineAnim;
    Animation crackedAnim;
    Animation shatteredAnim;
    Animation breakAnim;

    Vector2D position;
    int width, height;
    b2Vec2 direction;
};
