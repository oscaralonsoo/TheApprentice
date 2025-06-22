#pragma once

#include "PushableBox.h"
#include "Timer.h"
#include "IHookable.h"

class HookableBox : public PushableBox, public IHookable
{
public:
    HookableBox();
    virtual ~HookableBox();

    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;

    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;

    bool IsHookUsed() const { return hookUsed; }
    PhysBody* GetPhysBody() const { return pbody; }
    void Use();
    void ResetHook();

private:
    PhysBody* sensor = nullptr;
    Timer hookTimer;
    bool isHooking = false;
    float hookDuration = 350.0f;
    bool cancelledByProximity = false;
    bool hookUsed = false;
    float cancelDistanceThreshold = 30.0f;
};
