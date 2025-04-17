#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class CreeblerState {
    WALKING,
    DEAD
};

class Creebler : public Enemy
{
public:

	Creebler();
	~Creebler() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate() override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB);

    void Walk();


private:
    CreeblerState currentState = CreeblerState::WALKING;

    Animation walkAnim;
    Animation deadAnim;

    int direction = -1;
    float speed = 2.0f;
};
