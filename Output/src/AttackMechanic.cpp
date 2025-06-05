#include "AttackMechanic.h"
#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Physics.h"
#include "InvulnerabilitySystem.h"
#include "Audio.h"

void AttackMechanic::Init(Player* player) {
    this->player = player;

    soundAttackId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Slime/slime_attack.ogg", 1.0f);
}

void AttackMechanic::Update(float dt) {
    if (!player->GetMechanics()->GetMovementHandler()->CanAttack())
        return;
    if (!isAttacking) {
        bool attackPressed = false;

        // Teclado (J)
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {

            if (!attackSoundPlayed) {
                Engine::GetInstance().audio->PlayFx(soundAttackId, 1.0f, 0);
                attackSoundPlayed = true;
            }
            attackPressed = true;
        }

        // Mando (botón X)
        if (controller && SDL_GameControllerGetAttached(controller)) {
            bool xPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X) != 0;

            if (xPressed && !attackHeldPreviously) {
                attackPressed = true;

                if (!attackSoundPlayed) {
                    Engine::GetInstance().audio->PlayFx(soundAttackId, 1.0f, 0);
                    attackSoundPlayed = true;
                }
            }

            attackHeldPreviously = xPressed;
        }

        if (attackPressed) {
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
    int offsetX = (player->GetMechanics()->GetMovementDirection() > 0) ? 48 : -48;

    int playerX = METERS_TO_PIXELS(player->pbody->body->GetPosition().x) + offsetX;
    int playerY = METERS_TO_PIXELS(player->pbody->body->GetPosition().y) - 10;

    attackSensor = Engine::GetInstance().physics->CreateRectangleSensor(
        playerX, playerY,
        64, 64,
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

    attackDirection = player->GetMechanics()->GetMovementDirection();
    attackFlip = (attackDirection < 0);

    player->GetMechanics()->GetInvulnerabilitySystem()->StartInvulnerability();

    player->GetAnimation()->ForceSetState("attack");
}

void AttackMechanic::UpdateAttackSensor() {
    if (!attackSensor)
        return;

    int offsetX = (attackDirection > 0) ? 48 : -48; // Usa la dirección guardada, NO la actual

    int playerX = METERS_TO_PIXELS(player->pbody->body->GetPosition().x) + offsetX;
    int playerY = METERS_TO_PIXELS(player->pbody->body->GetPosition().y) - 10;

    b2Vec2 newPos(PIXEL_TO_METERS(playerX), PIXEL_TO_METERS(playerY));
    attackSensor->body->SetTransform(newPos, 0);
}

void AttackMechanic::DestroyAttackSensor() {
    if (attackSensor) {
        Engine::GetInstance().physics->DeletePhysBody(attackSensor);
        attackSensor = nullptr;
        attackSoundPlayed = false;
    }

    isAttacking = false;
    player->SetState("idle");
}

bool AttackMechanic::IsAttacking() const {
    return isAttacking;
}

void AttackMechanic::SetController(SDL_GameController* controller) {
    this->controller = controller;
}
