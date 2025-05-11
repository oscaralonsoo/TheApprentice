#include "HookManager.h"
#include "Engine.h"
#include "Scene.h"
#include "Player.h"
#include <algorithm>

void HookManager::RegisterHook(IHookable* hook)
{
    if (std::find(hooksInRange.begin(), hooksInRange.end(), hook) == hooksInRange.end())
        hooksInRange.push_back(hook);
}

void HookManager::UnregisterHook(IHookable* hook)
{
    hooksInRange.erase(std::remove(hooksInRange.begin(), hooksInRange.end(), hook), hooksInRange.end());
}

void HookManager::TryUseClosestHook()
{
    if (hooksInRange.empty()) return;

    Player* player = Engine::GetInstance().scene->GetPlayer();
    if (!player || !player->pbody || !player->pbody->body) return;

    b2Vec2 playerPos = player->pbody->body->GetPosition();

    std::sort(hooksInRange.begin(), hooksInRange.end(),
        [playerPos](IHookable* a, IHookable* b) {
            b2Vec2 aPos = a->GetPhysBody()->body->GetPosition();
            b2Vec2 bPos = b->GetPhysBody()->body->GetPosition();
            return b2Distance(playerPos, aPos) < b2Distance(playerPos, bPos);
        });

    for (IHookable* hook : hooksInRange)
    {
        if (!hook->IsHookUsed())
        {
            hook->Use();
            break;
        }
    }
}
