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

    // Cambiar de animación si se pide otro estado
    if (state != currentState && animations.find(state) != animations.end()) {
        currentAnimation = &animations[state];

        if (!currentAnimation->IsLoop()) {
            currentAnimation->Reset();
        }

        currentState = state;
    }

    bool adjustedFlip = flip;

    // Ajustes visuales según el estado
    if (state == "wall_slide") adjustedFlip = !flip;
    if (state == "push") adjustedFlip = !flip;

    if (state == "push" && adjustedFlip) drawX = x - 80;
    else if (state == "push") drawX = x - 10;
    if (state == "push") drawY = y - 10;
    if (state == "run_right") drawY = y - 19;
    if (state == "idle") drawY = y - 4;
    if (state == "attack") drawY = y - 20;
    if (state == "eat") drawY = y - 10;
    if (state == "hit") drawY = y - 10;
    if (state == "landing_stun") drawY = y + 18;
    if (state == "landing") drawY = y + 5;
    if (state == "transition") drawY = y - 100;
    if (state == "transition") drawX = x - 50;

    // Dibujar la animación actual
    if (currentAnimation) {
        Engine::GetInstance().render->DrawTexture(
            texture,
            drawX,
            drawY,
            &currentAnimation->GetCurrentFrame(),
            1.0f,
            0.0,
            INT_MAX,
            INT_MAX,
            adjustedFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
        );

        currentAnimation->Update();

        if (overlayActive && overlayAnimation) {
            int overlayOffsetX = 0, overlayOffsetY = 0;
            GetAnimationOffset(overlayState, overlayOffsetX, overlayOffsetY);

            Engine::GetInstance().render->DrawTexture(
                texture,
                drawX + overlayOffsetX,
                drawY + overlayOffsetY + 20,
                &overlayAnimation->GetCurrentFrame(),
                1.0f,
                0.0,
                INT_MAX,
                INT_MAX,
                adjustedFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
            );

            overlayAnimation->Update();

            if (overlayAnimation->HasFinished()) {
                ClearOverlayState();
            }
        }
    }
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
    if (state == "landing_stun") return AnimationStatePriority::LANDING_STUN;
    if (state == "hit") return AnimationStatePriority::HIT;
    if (state == "die") return AnimationStatePriority::DIE;
    if (state == "liana") return AnimationStatePriority::LIANA;
    if (state == "landing") return AnimationStatePriority::LANDING;
    if (state == "transition") return AnimationStatePriority::TRANSITION;
    if (state == "push") return AnimationStatePriority::PUSH;

    return AnimationStatePriority::IDLE;
}

void PlayerAnimation::SetPlayer(Player* p) {
    player = p;
}

void PlayerAnimation::ForceSetState(const std::string& newState) {
    currentAnimation = &animations[newState];
    currentState = newState;
    if (!currentAnimation->IsLoop()) {
        currentAnimation->Reset();
    }
    if (player) {
        player->SetState(newState);
    }
}

void PlayerAnimation::SetOverlayState(const std::string& state) {
    if (animations.find(state) != animations.end()) {
        overlayAnimation = &animations[state];
        overlayState = state;
        overlayAnimation->Reset();
        overlayActive = true;
    }
}

void PlayerAnimation::ClearOverlayState() {
    overlayAnimation = nullptr;
    overlayState = "";
    overlayActive = false;
}

void PlayerAnimation::GetAnimationOffset(const std::string& state, int& offsetX, int& offsetY) const {
    offsetX = 0;
    offsetY = 0;

    if (state == "push") {
        offsetX = -10;
        offsetY = -10;
    }
    else if (state == "push" && currentState == "push") {
        offsetX = -80;
    }
    else if (state == "run_right") {
        offsetY = -19;
    }
    else if (state == "idle") {
        offsetY = -4;
    }
    else if (state == "attack") {
        offsetY = -20;
    }
    else if (state == "eat" || state == "hit") {
        offsetY = -10;
    }
    else if (state == "landing_stun") {
        offsetY = +18;
    }
    else if (state == "landing") {
        offsetY = +5;
    }
    else if (state == "transition") {
        offsetX = -50;
        offsetY = -100;
    }
}