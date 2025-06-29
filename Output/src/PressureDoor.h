
#pragma once
#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Timer.h"

struct SDL_Texture;

enum class PressureDoorState {
    DISABLE, ENABLE, IDLE
};

class PressureDoor : public Entity {
public:
    PressureDoor();
    virtual ~PressureDoor();

    bool Awake();

    bool Start();

    bool Update(float dt);

    bool PostUpdate();

    bool CleanUp();

    void RenderTexture();

    void SetOpen(bool isOpen);

    bool IsOverlapping();

    void OnCollision(PhysBody* physA, PhysBody* physB);
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void CheckStartState();

public:
    int texW, texH;
    int height, width;
    int id;
    bool isOpen;
    bool shouldBeOpen;
    bool triggeredOnce = false;
    PressureDoorState state = PressureDoorState::IDLE;
private:
    PhysBody* pbody;
    SDL_Texture* texture;


    Animation disabledAnim;
    Animation enabledAnim;
    Animation idleAnim;
    Animation* currentAnimation = nullptr;

    bool isHorizontal = false;
};
