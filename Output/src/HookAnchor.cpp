#include "HookAnchor.h"
#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Physics.h"
#include "Log.h"
#include "Scene.h"

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
    pbody = Engine::GetInstance().physics->CreateRectangle(centerX, centerY, width, height, STATIC);
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


    return true;
}

bool HookAnchor::Update(float dt)
{
    if (texture)
        Engine::GetInstance().render->DrawTexture(texture, position.getX(), position.getY());
    if (playerInRange)
    {
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_L) == KEY_DOWN && !hookUsed && !isHooking)
        {
            Player* player = Engine::GetInstance().scene->GetPlayer();

            if (player && player->pbody && player->pbody->body)
            {
                hookUsed = true; // ← SOLO si va a impulsarse

                b2Vec2 playerPos = player->pbody->body->GetPosition();
                b2Vec2 hookPos = pbody->body->GetPosition();

                b2Vec2 direction = hookPos - playerPos;
                direction.Normalize();

                float impulseStrength = 30.0f;
                b2Vec2 impulse = impulseStrength * direction;

                player->pbody->body->SetLinearVelocity(impulse);
                player->GetMechanics()->GetMovementHandler()->SetCantMove(true);
                player->pbody->body->SetGravityScale(0.0f);

                isHooking = true;
                hookTimer.Start();
            }
        }
    }

    if (isHooking && hookTimer.ReadMSec() >= hookDuration)
    {
        LOG("Gancho cancelado por tiempo");
        EndHook();
    }

    if (isHooking && !cancelledByProximity)
    {
        Player* player = Engine::GetInstance().scene->GetPlayer();
        if (player && player->pbody && player->pbody->body)
        {
            b2Vec2 playerPos = player->pbody->body->GetPosition();
            b2Vec2 hookPos = pbody->body->GetPosition();

            float dx = METERS_TO_PIXELS(hookPos.x - playerPos.x);
            float dy = METERS_TO_PIXELS(hookPos.y - playerPos.y);
            float distance = sqrtf(dx * dx + dy * dy);

            if (distance <= cancelDistanceThreshold)
            {
                LOG("Gancho cancelado por proximidad");
                cancelledByProximity = true;
                EndHook();
            }
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
        playerInRange = true;
        cancelledByProximity = false;
    }
}


void HookAnchor::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    // Por ahora simplemente logueamos
    if (physB->ctype == ColliderType::PLAYER)
    {
        playerInRange = false;
    }
}

void HookAnchor::EndHook()
{
    Player* player = Engine::GetInstance().scene->GetPlayer();
    if (player && player->pbody)
    {
        player->GetMechanics()->GetMovementHandler()->SetCantMove(false);
        player->pbody->body->SetGravityScale(2.0f);
    }

    isHooking = false;
    hookUsed = false;
    cancelledByProximity = false;
}
