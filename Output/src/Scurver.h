#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

enum class ScurverState {
    IDLE,
    ATTACK,
    SLIDE,
    DEAD
};

class Scurver : public Enemy
{
public:

	Scurver();
	~Scurver() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate() override;
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB);

    void Attack(float dt);
    void Slide(float dt);


private:
    ScurverState currentState = ScurverState::IDLE;

    Animation attackAnim;
    Animation deadAnim;

    float previousDirection;
    Timer timer;
};
