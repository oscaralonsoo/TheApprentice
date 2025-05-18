#pragma once
#include "Enemy.h"
#include "SDL2/SDL.h"
#include "Physics.h"

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
    void OnCollision(PhysBody* physA, PhysBody* physB);
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void CheckState(float dt);
    void Idle();
    void Angry();
    void Throw(float dt);
    void BallMode();
    void ChangeBodyType();
    int CheckPuzzleState();
    void Bounce();
private:
    bool hasThrown = false;
    bool isDynamic = false;
    bool hasLaunched = false;

    float throwSpeed = 15.0f; 
    float ballModeSpeed = 17.0f;

    Vector2D playerPos;
    DungBeetleState currentState = DungBeetleState::IDLE;

    Animation idleAnim;
    Animation angryAnim;
    Animation throwingAnim;
    Animation ballModeAnim;
    Animation hitAnim;
    Animation dieAnim;
};