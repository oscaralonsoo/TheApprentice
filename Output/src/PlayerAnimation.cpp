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

void PlayerAnimation::Update(float dt, const std::string& state, int x, int y, bool visible) {
    if (animations.find(state) != animations.end()) {
        currentAnimation = &animations[state];
        //printf("Animación actual: %s\n", state.c_str());
    }

    if (visible) {
        Engine::GetInstance().render.get()->DrawTexture(texture, (int)x, (int)y, &GetCurrentFrame());
    }

    currentAnimation->Update();
}

const SDL_Rect& PlayerAnimation::GetCurrentFrame() const {
    return currentAnimation->GetCurrentFrame();
}
