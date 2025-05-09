#pragma once

class PhysBody;

class IHookable {
public:
    virtual ~IHookable() = default;

    virtual bool IsHookUsed() const = 0;
    virtual void Use() = 0;
    virtual void ResetHook() = 0;
    virtual PhysBody* GetPhysBody() const = 0;
};
