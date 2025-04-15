#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class HypnoviperState {
    SLEEPING,
    HITTED,
    DEAD
};

class Hypnoviper : public Enemy
{
public:

	Hypnoviper();
	~Hypnoviper() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate() override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB);


private:
    HypnoviperState currentState = HypnoviperState::SLEEPING;

    Animation sleepAnim;
    Animation hitAnim;
    Animation deadAnim;

    Timer hitTimer;
};
