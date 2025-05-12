#pragma once
#include "Enemy.h"
#include "SDL2/SDL.h"

enum class DungBeetleState {
	IDLE,
    ANGRY,
    THROW,
    BALLMODE,
	HIT,
	DEAD,
};

class DungBeetle : public Enemy {
public:
    DungBeetle();
    ~DungBeetle() override;

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate();
    bool CleanUp() override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void CheckState(float dt);
    void Idle();
    void Angry();
    void Throw(float dt);
    void BallMode();
private:
    bool hasThrown = false;

    Vector2D playerPos;
    DungBeetleState currentState = DungBeetleState::IDLE;

    Animation idleAnim;
    Animation angryAnim;
    Animation throwingAnim;
    Animation ballModeAnim;
    Animation hitAnim;
    Animation dieAnim;
};