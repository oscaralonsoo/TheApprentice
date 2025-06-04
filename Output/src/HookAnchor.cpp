#include "HookAnchor.h"
#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Physics.h"
#include "Log.h"
#include "Scene.h"
#include "Player.h"
#include "Audio.h"

HookAnchor::HookAnchor() : Entity(EntityType::HOOK_ANCHOR) {}

HookAnchor::~HookAnchor() {}

bool HookAnchor::Awake() { return true; }

bool HookAnchor::Start()
{
    int centerX = position.getX() + width / 2;
    int centerY = position.getY() + height / 2;
    // Sensor circular alrededor del punto de anclaje
    float radius = std::max(width, height) * 10.0f;

    // Collider principal
    pbody = Engine::GetInstance().physics->CreateRectangleSensor(centerX, centerY, width, height, STATIC, CATEGORY_HOOK, CATEGORY_PLAYER);
    pbody->ctype = ColliderType::HOOK_ANCHOR;
    pbody->listener = this;

    sensor = Engine::GetInstance().physics->CreateCircleSensor(
        centerX, centerY, radius,
        STATIC,
        CATEGORY_HOOK_SENSOR,   // categoría propia
        CATEGORY_PLAYER         // solo colisiona con el jugador
    );
    sensor->ctype = ColliderType::HOOK_SENSOR;
    sensor->listener = this;

    highlightTexture = Engine::GetInstance().textures->Load("Assets/Props/gancho_seleccionado.png");

    soundSpiderId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Slime/slime_spiderweb.ogg", 1.0f);

    return true;
}

bool HookAnchor::Update(float dt)
{
    Player* player = Engine::GetInstance().scene->GetPlayer();


    if (texture)
        Engine::GetInstance().render->DrawTexture(texture, position.getX(), position.getY());

    if (isHooking)
    {
        if (player && player->pbody && player->pbody->body)
        {
            if (player->GetState() != "hook") {
                player->GetAnimation()->SetOverlayState("transition");
                player->GetAnimation()->SetStateIfHigherPriority("hook");
            }
            b2Vec2 playerPos = player->pbody->body->GetPosition();
            b2Vec2 hookPos = pbody->body->GetPosition();

            int x1 = METERS_TO_PIXELS(playerPos.x);
            int y1 = METERS_TO_PIXELS(playerPos.y);
            int x2 = METERS_TO_PIXELS(hookPos.x);
            int y2 = METERS_TO_PIXELS(hookPos.y);

            Engine::GetInstance().render->DrawLine(x1, y1, x2, y2, 255, 255, 255, 255);
        }

        if (player && !wasOnGroundAtHookStart && player->GetMechanics()->IsOnGround())
        {
            EndHook();
            return true;
        }

        if (hookTimer.ReadMSec() >= hookDuration)
        {
            EndHook();
        }

        if (!cancelledByProximity)
        {
            b2Vec2 playerPos = player->pbody->body->GetPosition();
            b2Vec2 hookPos = pbody->body->GetPosition();

            float dx = METERS_TO_PIXELS(hookPos.x - playerPos.x);
            float dy = METERS_TO_PIXELS(hookPos.y - playerPos.y);
            float distance = sqrtf(dx * dx + dy * dy);

            if (distance <= cancelDistanceThreshold)
            {
                cancelledByProximity = true;
                EndHook();
            }
        }
    }

    // Dibujar un recuadro sobre el gancho si es el activo
    Scene* scene = Engine::GetInstance().scene.get();
    if (!player || !player->GetMechanics()->GetMovementHandler()->IsHookUnlocked())
        return true; // No dibuja el marco si no está desbloqueado

    IHookable* closest = scene->GetHookManager()->GetClosestHook();
    if (closest == this)
    {
        int borderX = position.getX();
        int borderY = position.getY();

        if (highlightTexture) {
            Engine::GetInstance().render->DrawTexture(highlightTexture, borderX, borderY);
        }
    }

    return true;
}

bool HookAnchor::CleanUp()
{
    if (pbody)
    {
        Engine::GetInstance().physics->DeletePhysBody(pbody);
        pbody = nullptr;
    }

    if (sensor)
    {
        Engine::GetInstance().physics->DeletePhysBody(sensor);
        sensor = nullptr;
    }

    if (highlightTexture) {
        SDL_DestroyTexture(highlightTexture);
        highlightTexture = nullptr;
    }

    return true;
}

void HookAnchor::SetParameters(pugi::xml_node parameters)
{
    position.x = parameters.attribute("x").as_int();
    position.y = parameters.attribute("y").as_int();
    width = parameters.attribute("w").as_int();
    height = parameters.attribute("h").as_int();

    std::string texturePath = parameters.attribute("texture").as_string();
    texture = Engine::GetInstance().textures->Load(texturePath.c_str());
}

void HookAnchor::SetPosition(Vector2D pos)
{
    position = pos;
    b2Vec2 bodyPos(PIXEL_TO_METERS(pos.getX() + width / 2), PIXEL_TO_METERS(pos.getY() + height / 2));
    if (pbody && pbody->body)
        pbody->body->SetTransform(bodyPos, 0);
}

Vector2D HookAnchor::GetPosition() const
{
    return position;
}

void HookAnchor::OnCollision(PhysBody* physA, PhysBody* physB)
{
    if (physA->ctype == ColliderType::HOOK_SENSOR && physB->ctype == ColliderType::PLAYER)
    {
        hookUsed = false;
        Engine::GetInstance().scene->GetHookManager()->RegisterHook(this);
    }
}

void HookAnchor::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    if (physB->ctype == ColliderType::PLAYER)
    {
        Scene* scene = Engine::GetInstance().scene.get();
        if (scene && scene->GetHookManager())
        {
            scene->GetHookManager()->UnregisterHook(this);

            // Desactiva el gancho si este era el activo
            if (scene->GetActiveHook() == this)
            {
                scene->SetActiveHook(nullptr);
            }
        }
    }
}

void HookAnchor::EndHook()
{
    Player* player = Engine::GetInstance().scene->GetPlayer();
    if (player && player->pbody)
    {
        player->GetMechanics()->GetMovementHandler()->SetCantMove(false);
        player->pbody->body->SetGravityScale(2.0f);
        player->pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
    }

    isHooking = false;
    cancelledByProximity = false;
}

bool HookAnchor::IsPlayerWithinSensorRadius() const
{
    Player* player = Engine::GetInstance().scene->GetPlayer();
    if (!player || !player->pbody || !player->pbody->body || !sensor || !sensor->body)
    {
        return false;
    }

    b2Vec2 playerPos = player->pbody->body->GetPosition();
    b2Vec2 sensorPos = sensor->body->GetPosition();

    float dx = sensorPos.x - playerPos.x;
    float dy = sensorPos.y - playerPos.y;
    float distance = sqrtf(dx * dx + dy * dy);

    float sensorRadius = PIXEL_TO_METERS(std::max(width, height) * 10.0f); // radio en metros

    return distance <= sensorRadius;
}

void HookAnchor::ResetHook()
{
    hookUsed = false;
}

void HookAnchor::Use()
{
    Player* player = Engine::GetInstance().scene->GetPlayer();
    Engine::GetInstance().scene->SetActiveHook(this);

    if (player && player->pbody && player->pbody->body)
    {
        Player* player = Engine::GetInstance().scene->GetPlayer();
        wasOnGroundAtHookStart = player->GetMechanics()->IsOnGround(); // Guarda el estado del suelo

        hookUsed = true;

        b2Vec2 playerPos = player->pbody->body->GetPosition();
        b2Vec2 hookPos = pbody->body->GetPosition();

        b2Vec2 direction = hookPos - playerPos;
        float distance = direction.Length();
        direction.Normalize();

        float desiredTimeToReach = hookDuration / 1000.0f; // Usa tu duración real en segundos
        float requiredSpeed = distance / desiredTimeToReach;

        b2Vec2 impulse = requiredSpeed * direction;
        player->pbody->body->SetLinearVelocity(impulse);
        player->GetMechanics()->GetMovementHandler()->SetCantMove(true);
        player->pbody->body->SetGravityScale(0.0f);

        Engine::GetInstance().audio->PlayFx(soundSpiderId, 1.0f, 0);
        isHooking = true;
        hookTimer.Start();

        LOG("Intentando usar hook");
    }
}

