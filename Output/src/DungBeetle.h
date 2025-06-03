#pragma once
#include "Enemy.h"
#include "SDL2/SDL.h"
#include "DungBeetleBall.h"
#include "Physics.h"
#include <vector>

enum class DungBeetleState {
	IDLE,
    ANGRY,
    THROW,
    BALLMODE,
	HIT,
};
class DungBeetleBall;

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
    void Hit();
    void ChangeBodyType();
    int CheckPuzzleState();
    void Bounce();
    void ChangeColliderRadius(float newRadius, bool isSensor);
    void CollisionNavigationLayer();

    void AssignBalls(DungBeetleBall* ball);
private:
    bool hasThrown = false;
    bool isDynamic = false;
    bool hasLaunched = false;
    float firstBallTimer = 0.0f;
    bool firstBallLaunched = false;
    int ballsThrown = 0;
    int currentStatePuzzle = 0;
    int lastPuzzleState = 0;
    float throwSpeed = 19.0f; 
    float ballModeSpeed = 20.0f;
    float time= 0.0;
    Vector2D playerPos;
    Vector2D currentTileMap;
    DungBeetleState currentState = DungBeetleState::IDLE;

    Animation idleAnim;
    Animation angryAnim;
    Animation throwingAnim;
    Animation ballModeAnim;
    Animation ballAnim;
    Animation deathAnim;

    std::vector<DungBeetleBall*> dungBalls;
};