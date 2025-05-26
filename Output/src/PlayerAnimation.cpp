#include "PlayerAnimation.h"
#include "Engine.h"
#include "Render.h"

PlayerAnimation::PlayerAnimation() : currentAnimation(nullptr) {}

void PlayerAnimation::LoadAnimations(const pugi::xml_node& parameters, SDL_Texture* texture) {
    this->texture = texture;
    for (pugi::xml_node anim = parameters.first_child(); anim; anim = anim.next_sibling()) {
        std::string animName = anim.name();
        animations[animName].LoadAnimations(anim);
    }
    currentAnimation = &animations["idle"];
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