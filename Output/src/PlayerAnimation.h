#ifndef PLAYER_ANIMATION_H
#define PLAYER_ANIMATION_H

#include <unordered_map>
#include <string>
#include "Animation.h"
#include "pugixml.hpp"

struct SDL_Texture;

class PlayerAnimation {
public:
    PlayerAnimation();
    void LoadAnimations(const pugi::xml_node& parameters, SDL_Texture* texture);
    void Update(float dt, const std::string& state, int x, int y, bool visible = true);
    const SDL_Rect& GetCurrentFrame() const;

private:
    SDL_Texture* texture = NULL;
    std::unordered_map<std::string, Animation> animations;
    Animation* currentAnimation;
};

#endif // PLAYER_ANIMATION_H
