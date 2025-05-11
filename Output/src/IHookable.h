#pragma once

class PhysBody;

class IHookable {
public:
    virtual ~IHookable() = default;

    virtual bool IsHookUsed() const = 0;
    virtual void Use() = 0;
    virtual void ResetHook() = 0;
    virtual PhysBody* GetPhysBody() const = 0;
    virtual Vector2D GetRenderPosition() const = 0;
    virtual int GetRenderWidth() const = 0;
    virtual int GetRenderHeight() const = 0;
};
