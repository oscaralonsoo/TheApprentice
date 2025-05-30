#include "PlayerAnimation.h"
#include "Engine.h"
#include "Render.h"
#include "Log.h"

PlayerAnimation::PlayerAnimation() : currentAnimation(nullptr) {}

void PlayerAnimation::LoadAnimations(const pugi::xml_node& parameters, SDL_Texture* texture) {
    this->texture = texture;

    for (pugi::xml_node anim = parameters.first_child(); anim; anim = anim.next_sibling()) {
        std::string animName = anim.name();
        animations[animName].LoadAnimations(anim);
    }

    if (animations.empty()) {
        LOG("❌ No se cargaron animaciones.");
        currentAnimation = nullptr;
        return;
    }

    auto it = animations.find("idle");
    if (it != animations.end()) {
        currentAnimation = &it->second;
        currentState = "idle";
    }
    else {
        LOG("⚠ Animación 'idle' no encontrada. Asignando la primera animación cargada.");
        currentAnimation = &animations.begin()->second;
        currentState = animations.begin()->first;
    }
}

void PlayerAnimation::Update(const std::string& state, int x, int y, bool visible, bool flip)
{
    int drawX = x;
    int drawY = y;

    if (state != currentState && animations.find(state) != animations.end()) {
        currentAnimation = &animations[state];

        if (!currentAnimation->IsLoop()) {
            currentAnimation->Reset();
        }

        currentState = state;
    }

    bool adjustedFlip = flip;

    // Si es la animaci�n de wall_slide, invertimos el flip
    if (state == "wall_slide") {
        adjustedFlip = !flip;
    
    }

    if (state == "push") {
        adjustedFlip = !flip;
    }

    if (state == "push" && adjustedFlip) {  // no está flippeado -> mirando a la derecha
        drawX = x - 80; // desplaza 10 píxeles a la izquierda para el sprite
    }
    else if (state == "push" && !adjustedFlip) {  // no está flippeado -> mirando a la derecha
        drawX = x - 10; // desplaza 10 píxeles a la izquierda para el sprite
    }
    if (state == "push") {  // no está flippeado -> mirando a la derecha
        drawY = y - 10; // desplaza 10 píxeles a la izquierda para el sprite
    }
    if (state == "run_right") {  // no está flippeado -> mirando a la derecha
        drawY = y - 15; // desplaza 10 píxeles a la izquierda para el sprite
    }
    if (state == "attack") {  // no está flippeado -> mirando a la derecha
        drawY = y - 5; // desplaza 10 píxeles a la izquierda para el sprite
    }
    if (state == "eat") {  // no está flippeado -> mirando a la derecha
        drawY = y - 10; // desplaza 10 píxeles a la izquierda para el sprite
    }
    if (state == "hit") {  // no está flippeado -> mirando a la derecha
        drawY = y - 10; // desplaza 10 píxeles a la izquierda para el sprite
    }

    Engine::GetInstance().render->DrawTexture(
        texture,
        drawX,
        drawY,
        &GetCurrentFrame(),
        1.0f,
        0.0,
        INT_MAX,
        INT_MAX,
        adjustedFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
    );

    currentAnimation->Update();
}

const SDL_Rect& PlayerAnimation::GetCurrentFrame() const {
    return currentAnimation->GetCurrentFrame();
}

bool PlayerAnimation::HasFinished() const {
    return currentAnimation && currentAnimation->HasFinished();
}

std::string PlayerAnimation::GetCurrentState() const {
    return currentState;
}

int PlayerAnimation::GetLoopCount() const {
    return currentAnimation ? currentAnimation->GetLoopCount() : 0;
}

void PlayerAnimation::SetStateIfHigherPriority(const std::string& newState) {
    if (newState == currentState) return;

    auto newPriority = GetPriorityForState(newState);
    auto currentPriority = GetPriorityForState(currentState);

    if (static_cast<int>(newPriority) >= static_cast<int>(currentPriority) ||
        (currentAnimation && currentAnimation->HasFinished())) {

        currentAnimation = &animations[newState];
        currentState = newState;

        if (!currentAnimation->IsLoop()) {
            currentAnimation->Reset();
        }

        if (player) {
            player->SetState(newState); //  ACTUALIZA EL STATE DEL PLAYER
        }
    }
}

AnimationStatePriority PlayerAnimation::GetPriorityForState(const std::string& state) const {
    if (state == "idle") return AnimationStatePriority::IDLE;
    if (state == "run_right") return AnimationStatePriority::RUN_RIGHT;
    if (state == "fall") return AnimationStatePriority::FALL;
    if (state == "jump") return AnimationStatePriority::JUMP;
    if (state == "glide") return AnimationStatePriority::GLIDE;
    if (state == "wall_slide") return AnimationStatePriority::WALL_SLIDE;
    if (state == "doublejump") return AnimationStatePriority::DOUBLEJUMP;
    if (state == "walljump") return AnimationStatePriority::WALLJUMP;
    if (state == "dash") return AnimationStatePriority::DASH;
    if (state == "attack") return AnimationStatePriority::ATTACK;
    if (state == "eat") return AnimationStatePriority::EAT;
    if (state == "hit") return AnimationStatePriority::HIT;
    if (state == "die") return AnimationStatePriority::DIE;

    return AnimationStatePriority::IDLE;
}

void PlayerAnimation::SetPlayer(Player* p) {
    player = p;
}