#include "WallSlideMechanic.h"
#include "Player.h"
#include "Engine.h"
#include "Physics.h"

void WallSlideMechanic::Init(Player* player) {
    this->player = player;
}

void WallSlideMechanic::Update(float dt) {

    if (player->GetMechanics()->GetMovementHandler()->IsWallSlideCooldownActive())
        return; 

    if (player->GetMechanics()->IsTouchingWall() && !wallSlideCooldownActive) {
        StartWallSlide();
    }
    else if (!player->GetMechanics()->IsTouchingWall()) {
        StopWallSlide();
    }
}

void WallSlideMechanic::StartWallSlide() {
    if (!isWallSliding) {
        isWallSliding = true;
        player->SetState("wall_slide");

        player->pbody->body->SetGravityScale(8.0f);
        player->pbody->body->SetLinearVelocity(b2Vec2_zero);
    }
}

void WallSlideMechanic::StopWallSlide() {
    if (isWallSliding) {
        isWallSliding = false;
        player->pbody->body->SetGravityScale(2.0f); // volver a gravedad normal

        wallSlideCooldownTimer.Start();
        wallSlideCooldownActive = true;
    }
}

void WallSlideMechanic::OnTouchWall(int direction) {
    player->GetMechanics()->SetIsTouchingWall(true);
    player->GetMechanics()->SetWallSlideDirection(direction); // aquí guardamos el dato
    wallSlideCooldownActive = false;
}

void WallSlideMechanic::OnLeaveWall() {
    player->GetMechanics()->SetIsTouchingWall(false);
    wallSlideCooldownTimer.Start();
    wallSlideCooldownActive = true;
}
