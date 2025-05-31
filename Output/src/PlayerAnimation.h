#ifndef PLAYER_ANIMATION_H
#define PLAYER_ANIMATION_H

#include <unordered_map>
#include <string>
#include "Animation.h"
#include "pugixml.hpp"
#include "Player.h"

enum class AnimationStatePriority {
    IDLE = 0,
    RUN_RIGHT = 1,
    LIANA = 2,
    FALL = 3,
    JUMP = 4,
    WALL_SLIDE = 5,
    DOUBLEJUMP = 6,
    WALLJUMP = 7,
    DASH = 8,
    GLIDE = 9,
    ATTACK = 10,
    EAT = 11,
    LANDING_STUN = 12,
    HIT = 13,
    DIE = 14
};

struct SDL_Texture;

class Player;

class PlayerAnimation {
public:
    PlayerAnimation();
    void LoadAnimations(const pugi::xml_node& parameters, SDL_Texture* texture);
    void Update(const std::string& state, int x, int y, bool visible = true, bool flip = false);
    const SDL_Rect& GetCurrentFrame() const;
    bool HasFinished() const;
    std::string GetCurrentState() const;
    int GetLoopCount() const;
    void SetStateIfHigherPriority(const std::string& newState);
    AnimationStatePriority GetPriorityForState(const std::string& state) const;
    void SetPlayer(Player* p);
    void ForceSetState(const std::string& newState);

private:
    SDL_Texture* texture = NULL;
    std::unordered_map<std::string, Animation> animations;
    Animation* currentAnimation;
    std::string currentState = "idle";
    Player* player = nullptr;
};

#endif // PLAYER_ANIMATION_H
