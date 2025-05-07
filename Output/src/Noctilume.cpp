#include "Noctilume.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Entity.h"
#include "Log.h"
#include <cmath>

Noctilume::Noctilume() : Enemy(EntityType::NOCTILUME)
{
    waveOffset = static_cast<float>(rand() % 628) / 100.0f;
}

Noctilume::~Noctilume() = default;

bool Noctilume::Awake()
{
    return false;
}

bool Noctilume::Start()
{
    pugi::xml_document configDoc;
    if (!configDoc.load_file("config.xml")) return false;

    const std::string typeName = "Noctilume";
    for (auto node : configDoc.child("config").child("scene").child("animations").child("enemies").children("enemy")) {
        if (node.attribute("type").as_string() == typeName) {
            texture = Engine::GetInstance().textures->Load(node.attribute("texture").as_string());
            idleAnim.LoadAnimations(node.child("idle"));
            currentAnimation = &idleAnim;
            break;
        }
    }

    pbody = Engine::GetInstance().physics->CreateCircleSensor( static_cast<int>(position.getX() + texH / 2),
        static_cast<int>(position.getY() + texH / 2),texH / 4,bodyType::DYNAMIC);

    pbody->ctype = ColliderType::ENEMY;
    pbody->listener = this;

    if (!gravity) pbody->body->SetGravityScale(0);

    pathfinding = new Pathfinding();
    ResetPath();

    b2Fixture* fixture = pbody->body->GetFixtureList();
    if (fixture) {
        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_PLAYER_DAMAGE | CATEGORY_ATTACK;
        fixture->SetFilterData(filter);
    }

    return true;
}
bool Noctilume::Update(float dt)
{
        HandleStateTransition();
        TryInitiateDiveAttack(dt);

    switch (currentState)
    {
    case NoctilumeState::IDLE:
        IdleFlying(dt);
        break;
    case NoctilumeState::FLYING:
        Flying(dt);
        break;
    case NoctilumeState::DIVE:
        Dive(dt);
        break;
    case NoctilumeState::DEAD:
        break;
    }

    if (pbody)
    {
        pbody->body->SetTransform(
            b2Vec2(PIXEL_TO_METERS(position.x), PIXEL_TO_METERS(position.y)),
            0.0f
        );
    }

    return Enemy::Update(dt);
}
bool Noctilume::PostUpdate() {
    Enemy::PostUpdate();
    return true;
}
void Noctilume::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
    case ColliderType::ATTACK:
        break;
    }
}

bool Noctilume::CleanUp()
{
    return false;
}

void Noctilume::IdleFlying(float dt)
{
    timePassed += dt;

    const float horizontalRange = 500.0f;
    const float waveAmplitude = 4.5f;
    const float waveFrequency = 0.00035f;

    float pingPong = fmod(timePassed * waveFrequency * 2 * horizontalRange, 2 * horizontalRange);
    float x = (pingPong < horizontalRange) ? pingPong : (2 * horizontalRange - pingPong);

    position.x = 1200 + x;
    position.y = 500 + sin(timePassed * waveFrequency + waveOffset) * waveAmplitude;
}

void Noctilume::Flying(float dt)
{
    timePassed += dt;

    const Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();
    Vector2D noctPos = (pbody)
        ? Vector2D{ static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().x)),
                    static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().y)) }
    : position;

    delayedPlayerX += (playerPos.x - delayedPlayerX) * 0.02f;
    delayedPlayerY += (playerPos.y - delayedPlayerY) * 0.02f;

    const float hoverHeight = 300.0f;
    noctPos.y += (delayedPlayerY - hoverHeight - noctPos.y) * 0.5f;

    const float oscillationAmplitude = 200.0f;
    const float oscillationSpeed = 0.002f;

    float sinValue = sin(oscillationSpeed * timePassed + 2.0f);
    noctPos.x = delayedPlayerX + oscillationAmplitude * sinValue;

    lastSinValue = sinValue;
    position = noctPos;
}
void Noctilume::Dive(float dt)
{
    timePassed += dt;

    const float horizontalDistance = fabs(diveTargetPos.x - diveStartPos.x);
    const float diveDuration = std::max(800.0f, 800.0f + horizontalDistance * 0.4f);

    diveElapsedTime += dt;
    float t = diveElapsedTime / diveDuration;

    if (t >= 1.0f)
    {
        position = diveTargetPos; // asegúrate de que llega al final
        currentState = NoctilumeState::FLYING;
        diveElapsedTime = 0.0f;
        return;
    }

    float u = 1.0f - t;

    // Bézier cuadrática: p0*(1-t)^2 + p1*2*(1-t)*t + p2*t^2
    position = diveStartPos * (u * u) + diveControlPos * (2 * u * t) + diveTargetPos * (t * t);
}


float Noctilume::DistanceToPlayer()
{
    auto playerPos = Engine::GetInstance().scene->GetPlayerPosition();
    float dx = playerPos.x - position.x;
    float dy = playerPos.y - position.y;
    return sqrt(dx * dx + dy * dy);
}
void Noctilume::HandleStateTransition()
{
    float distance = DistanceToPlayer();

    if (currentState == NoctilumeState::IDLE && distance <= 700.0f)
        currentState = NoctilumeState::FLYING;
    else if (currentState == NoctilumeState::FLYING && distance > 700.0f)
        currentState = NoctilumeState::IDLE;
}
void Noctilume::TryInitiateDiveAttack(float dt)
{
    float distance = DistanceToPlayer();

    if (currentState != NoctilumeState::FLYING || distance > 400.0f)
    {
        proximityTimer = 0.0f;
        return;
    }

    proximityTimer += dt;
    attackTimer += dt;

    if (proximityTimer >= proximityDuration && attackTimer >= attackCooldown)
    {
        const Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();
        const float horizontalOffset = 300.0f;  // más ancho para que sea más visible la parábola

        bool goingRight = lastSinValue > 0.0f;

        // Start y target son simétricos respecto al jugador
        diveStartPos = {
            playerPos.x + (goingRight ? -horizontalOffset : horizontalOffset),
            playerPos.y - 150.0f  // un poco más arriba que el jugador
        };

        diveTargetPos = {
            playerPos.x + (goingRight ? horizontalOffset : -horizontalOffset),
            playerPos.y - 150.0f
        };

        diveControlPos = playerPos; // el vértice de la parábola: el jugador

        currentState = NoctilumeState::DIVE;
        diveElapsedTime = attackTimer = proximityTimer = 0.0f;

        LOG("Noctilume starts dive towards player!");
    }
}




