#pragma once

#include <vector>
#include "HookAnchor.h"
#include "IHookable.h"

class HookManager
{
public:
    void RegisterHook(IHookable* hook);
    void UnregisterHook(IHookable* hook);
    void TryUseClosestHook();
    IHookable* GetClosestHook() const;
    bool IsHookVisible(IHookable* hook) const;
    void ClearHooks();
    void MarkHookAsUsed(IHookable* hook);
    void ResetUsedHooks();

private:
    std::vector<IHookable*> hooksInRange;
    std::vector<IHookable*> usedHooks;
};