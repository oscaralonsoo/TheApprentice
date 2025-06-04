#pragma once

#include "Timer.h"

class Player;

class WallSlideMechanic {
public:
    void Init(Player* player);
    void Update(float dt);

    void OnTouchWall(int direction);
    void OnLeaveWall();

private:
    void StartWallSlide();
    void StopWallSlide();

private:
    Player* player = nullptr;

    bool isWallSliding = false;

    Timer wallSlideCooldownTimer;
    float wallSlideCooldownTime = 3000.0f; // tiempo de enfriamiento para volver a engancharse
    bool wallSlideCooldownActive = false;

    int soundWalkId = 0;
};
