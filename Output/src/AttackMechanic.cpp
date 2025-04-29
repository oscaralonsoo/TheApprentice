#include "AttackMechanic.h"
#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Physics.h"

void AttackMechanic::Init(Player* player) {
    this->player = player;
}

void AttackMechanic::Update(float dt) {
    if (!isAttacking) {
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
            StartAttack();
        }
    }
    else {
        UpdateAttackSensor();

        if (attackTimer.ReadMSec() >= attackDuration) {
            DestroyAttackSensor();
        }
    }
}

void AttackMechanic::StartAttack() {
    int offsetX = (player->GetMechanics()->GetMovementDirection() > 0) ? 38 : -38;

    int playerX = METERS_TO_PIXELS(player->pbody->body->GetPosition().x) + offsetX;
    int playerY = METERS_TO_PIXELS(player->pbody->body->GetPosition().y) - 10;

    attackSensor = Engine::GetInstance().physics->CreateRectangleSensor(
        playerX, playerY,
        32, 64,
        KINEMATIC,
        CATEGORY_ATTACK,
        CATEGORY_ENEMY | CATEGORY_LIFE_PLANT
    );

    if (attackSensor) {
        attackSensor->ctype = ColliderType::ATTACK;
        attackSensor->listener = player;
    }

    isAttacking = true;
    attackTimer.Start();

    player->SetState("attack");
}

void AttackMechanic::UpdateAttackSensor() {
    if (!attackSensor)
        return;

    int offsetX = (player->GetMechanics()->GetMovementDirection() > 0) ? 38 : -38;
    int playerX = METERS_TO_PIXELS(player->pbody->body->GetPosition().x) + offsetX;
    int playerY = METERS_TO_PIXELS(player->pbody->body->GetPosition().y) - 10;

    b2Vec2 newPos(PIXEL_TO_METERS(playerX), PIXEL_TO_METERS(playerY));
    attackSensor->body->SetTransform(newPos, 0);
}

void AttackMechanic::DestroyAttackSensor() {
    if (attackSensor) {
        Engine::GetInstance().physics->DeletePhysBody(attackSensor);
        attackSensor = nullptr;
    }

    isAttacking = false;
    player->SetState("idle");
}

bool AttackMechanic::IsAttacking() const {
    return isAttacking;
}
