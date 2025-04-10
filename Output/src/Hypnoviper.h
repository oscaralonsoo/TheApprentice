#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class HypnoviperState {
    SLEEPING,
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
    bool CleanUp() override;
    void Sleep();
    void OnCollision(PhysBody* physA, PhysBody* physB) override;


private:
    HypnoviperState currentState = HypnoviperState::SLEEPING;

    Animation sleepAnim;

    float previousDirection;
    Timer timer;
};
