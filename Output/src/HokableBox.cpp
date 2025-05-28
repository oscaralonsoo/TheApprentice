#include "HokableBox.h"
#include "Engine.h"
#include "Render.h"
#include "Scene.h"
#include "Physics.h"
#include "Player.h"
#include "Log.h"

HookableBox::HookableBox() : PushableBox() {}

HookableBox::~HookableBox() {}

bool HookableBox::Start()
{
    PushableBox::Start(); // llama al base para crear el pbody

    float radius = std::max(width, height) * 10.0f;
    sensor = Engine::GetInstance().physics->CreateCircleSensor(
        position.getX() + width / 2,
        position.getY() + height / 2,
        radius,
        STATIC,
        CATEGORY_HOOK_SENSOR,
        CATEGORY_PLAYER
    );
    sensor->ctype = ColliderType::HOOK_SENSOR;
    sensor->listener = this;

    return true;
}

bool HookableBox::Update(float dt)
{
    PushableBox::Update(dt);

    if (isHooking)
    {
        b2Vec2 playerPos = Engine::GetInstance().scene->GetPlayer()->pbody->body->GetPosition();
        b2Vec2 boxPos = pbody->body->GetPosition();

        int x1 = METERS_TO_PIXELS(playerPos.x);
        int y1 = METERS_TO_PIXELS(playerPos.y);
        int x2 = METERS_TO_PIXELS(boxPos.x);
        int y2 = METERS_TO_PIXELS(boxPos.y);

        Engine::GetInstance().render->DrawLine(x1, y1, x2, y2, 255, 255, 255, 255);

        if (hookTimer.ReadMSec() >= hookDuration)
            isHooking = false;

        if (!cancelledByProximity)
        {
            float dx = x2 - x1;
            float dy = y2 - y1;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist <= cancelDistanceThreshold)
            {
                cancelledByProximity = true;
                isHooking = false;
            }
        }
    }

    return true;
}

bool HookableBox::CleanUp()
{
    if (sensor)
        Engine::GetInstance().physics->DeletePhysBody(sensor);

    return PushableBox::CleanUp();
}

void HookableBox::Use()
{
    hookUsed = true;
    isHooking = true;
    hookTimer.Start();

    b2Vec2 playerPos = Engine::GetInstance().scene->GetPlayer()->pbody->body->GetPosition();
    b2Vec2 boxPos = pbody->body->GetPosition();

    b2Vec2 direction = playerPos - boxPos;
    direction.Normalize();

    float force = 25.0f;
    pbody->body->ApplyLinearImpulseToCenter(force * direction, true);
}

void HookableBox::ResetHook()
{
    hookUsed = false;
}

void HookableBox::OnCollision(PhysBody* physA, PhysBody* physB)
{
    if (physA->ctype == ColliderType::HOOK_SENSOR && physB->ctype == ColliderType::PLAYER)
    {
        hookUsed = false;
        Engine::GetInstance().scene->GetHookManager()->RegisterHook(this);
    }
}

void HookableBox::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    if (physB->ctype == ColliderType::PLAYER)
    {
        Engine::GetInstance().scene->GetHookManager()->UnregisterHook(this);
    }
}
