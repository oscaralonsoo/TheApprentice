#ifndef PLAYER_ANIMATION_H
#define PLAYER_ANIMATION_H

#include <unordered_map>
#include <string>
#include "Animation.h"
#include "pugixml.hpp"
#include "Player.h"

enum class AnimationStatePriority {
    IDLE = 0,
    WALL_SLIDE = 1,
    FALL = 2,
    RUN_RIGHT = 3,
    LANDING = 4,
    LIANA = 5,
    JUMP = 6,
    DOUBLEJUMP = 7,
    WALLJUMP = 8,
    DASH = 9,
    GLIDE = 10,
    PUSH = 11,
    ATTACK = 12,
    EAT = 13,
    LANDING_STUN = 14,
    HIT = 15,
    DIE = 16,
    TRANSITION = 17
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
    void SetOverlayState(const std::string& state);
    void ClearOverlayState();
    void GetAnimationOffset(const std::string& state, int& offsetX, int& offsetY) const;

private:
    SDL_Texture* texture = NULL;
    std::unordered_map<std::string, Animation> animations;
    Animation* currentAnimation;
    std::string currentState = "idle";
    Player* player = nullptr;
    Animation* overlayAnimation = nullptr;
    std::string overlayState = "";
    bool overlayActive = false;
};

#endif // PLAYER_ANIMATION_H
