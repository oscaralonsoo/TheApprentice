#include "GodModeSystem.h"
#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Physics.h"

void GodModeSystem::Init(Player* player) {
    this->player = player;
}

void GodModeSystem::Update(float dt) {
    if (!godModeEnabled)
        return;

    b2Vec2 velocity = { 0.0f, 0.0f };

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
        velocity.y = -8.0f;
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
        velocity.y = 8.0f;
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
        velocity.x = -8.0f;
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
        velocity.x = 8.0f;
    }

    player->pbody->body->SetGravityScale(0.0f);

    player->pbody->body->SetLinearVelocity(velocity);
    player->SetState("idle");
}

void GodModeSystem::Toggle() {
    godModeEnabled = !godModeEnabled;
}

bool GodModeSystem::IsEnabled() const {
    return godModeEnabled;
}
