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

void PlayerAnimation::Update(float dt, const std::string& state, int x, int y, bool visible, bool flip)
{
    if (animations.find(state) != animations.end()) {
        currentAnimation = &animations[state];
        //printf("Animaci�n actual: %s\n", state.c_str());
    }

    bool adjustedFlip = flip;

    // Si es la animaci�n de wall_slide, invertimos el flip
    if (state == "wall_slide") {
        adjustedFlip = !flip;
    }

    Engine::GetInstance().render->DrawTexture(
        texture,
        x,
        y,
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
