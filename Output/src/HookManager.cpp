#include "HookManager.h"
#include "Engine.h"
#include "Scene.h"
#include "Player.h"
#include <algorithm>
#include "Physics.h"
#include "Log.h"

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
    Player* player = Engine::GetInstance().scene->GetPlayer();
    if (!player || !player->GetMechanics()->GetMovementHandler()->IsHookUnlocked())
        return;  // No permitido si no está desbloqueado

    if (hooksInRange.empty() || !player->pbody || !player->pbody->body)
        return;

    b2Vec2 playerPos = player->pbody->body->GetPosition();
    int playerDirection = player->GetMovementDirection();

    std::sort(hooksInRange.begin(), hooksInRange.end(),
        [playerPos](IHookable* a, IHookable* b) {
            b2Vec2 aPos = a->GetPhysBody()->body->GetPosition();
            b2Vec2 bPos = b->GetPhysBody()->body->GetPosition();
            return b2Distance(playerPos, aPos) < b2Distance(playerPos, bPos);
        });

    for (IHookable* hook : hooksInRange)
    {
        if (hook->IsHookUsed())
            continue;

        b2Vec2 hookPos = hook->GetPhysBody()->body->GetPosition();
        float horizontalDiff = hookPos.x - playerPos.x;

        if ((playerDirection == -1 && horizontalDiff > 0) || (playerDirection == 1 && horizontalDiff < 0))
            continue;

        if (IsHookVisible(hook))
        {
            hook->Use();
            break;
        }
    }
}

IHookable* HookManager::GetClosestHook() const
{
    if (hooksInRange.empty())
        return nullptr;

    Player* player = Engine::GetInstance().scene->GetPlayer();
    if (!player || !player->pbody || !player->pbody->body)
        return nullptr;

    b2Vec2 playerPos = player->pbody->body->GetPosition();
    int playerDirection = player->GetMovementDirection(); // -1 izquierda, 1 derecha

    IHookable* closest = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (IHookable* hook : hooksInRange)
    {
        if (hook->IsHookUsed())
            continue;

        b2Vec2 hookPos = hook->GetPhysBody()->body->GetPosition();
        float horizontalDiff = hookPos.x - playerPos.x;

        // Filtrar por dirección: si mira izquierda y el gancho está a la derecha, se salta
        if ((playerDirection == -1 && horizontalDiff > 0) || (playerDirection == 1 && horizontalDiff < 0))
            continue;

        if (!IsHookVisible(hook))
            continue;

        float distance = b2Distance(playerPos, hookPos);
        if (distance < closestDistance)
        {
            closest = hook;
            closestDistance = distance;
        }
    }

    return closest;
}

bool HookManager::IsHookVisible(IHookable* hook) const
{
    Player* player = Engine::GetInstance().scene->GetPlayer();
    if (!player || !player->pbody || !player->pbody->body || !hook || !hook->GetPhysBody() || !hook->GetPhysBody()->body)
        return false;

    b2Vec2 start = player->pbody->body->GetPosition();
    b2Vec2 end = hook->GetPhysBody()->body->GetPosition();

    // Convertir a píxeles y luego a tiles del mapa
    Vector2D playerMapPos = Engine::GetInstance().map->WorldToMap(METERS_TO_PIXELS(start.x), METERS_TO_PIXELS(start.y));
    Vector2D hookMapPos = Engine::GetInstance().map->WorldToMap(METERS_TO_PIXELS(end.x), METERS_TO_PIXELS(end.y));

    int x0 = (int)playerMapPos.getX();
    int y0 = (int)playerMapPos.getY();
    int x1 = (int)hookMapPos.getX();
    int y1 = (int)hookMapPos.getY();

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    MapLayer* navigationLayer = Engine::GetInstance().map->GetNavigationLayer();
    if (!navigationLayer)
        return true; // No capa de navegación, no bloqueamos

    while (true)
    {
        uint32_t tileId = navigationLayer->Get(x0, y0);

        // Si hay un tile sólido
        if (tileId != 0)
        {
            return false;
        }

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
    return true;
}

void HookManager::ClearHooks()
{
    hooksInRange.clear(); // limpia el set o lista de hooks registrados
}