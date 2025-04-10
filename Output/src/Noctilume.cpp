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

Noctilume::~Noctilume()
{
}

bool Noctilume::Awake()
{
    return false;
}

bool Noctilume::Start()
{
    pugi::xml_document configDoc;
    if (!configDoc.load_file("config.xml")) {
        return false;
    }

    std::string typeName = "Noctilume";
    for (pugi::xml_node node : configDoc.child("config").child("scene").child("animations").child("enemies").children("enemy")) {
        if (node.attribute("type").as_string() == typeName) {
            texture = Engine::GetInstance().textures->Load(node.attribute("texture").as_string());
            idleAnim.LoadAnimations(node.child("idle"));
            currentAnimation = &idleAnim;
            break;
        }
    }
    pbody = Engine::GetInstance().physics.get()->CreateCircleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

    pbody->ctype = ColliderType::ENEMY;
    pbody->listener = this;

    if (!gravity) pbody->body->SetGravityScale(0);

    pathfinding = new Pathfinding();
    ResetPath();

    return true;
}
bool Noctilume::Update(float dt)
{
    if (pathfinding->HasFoundPlayer()) {
        float distance = DistanceToPlayer();

        if (currentState == NoctilumeState::IDLE && distance <= 700) {
            currentState = NoctilumeState::FLYING;
        }
        else if (currentState == NoctilumeState::FLYING && distance > 700) {
            currentState = NoctilumeState::IDLE;
        }

        // Solo controla el ataque en modo FLYING
        if (currentState == NoctilumeState::FLYING) {
            if (distance <= 400.0f) {
                proximityTimer += dt;
                attackTimer += dt;

                if (proximityTimer >= proximityDuration && attackTimer >= attackCooldown) {
                    // Preparar dive
                    diveStartPos = position;
                    diveTargetPos = Engine::GetInstance().scene->GetPlayerPosition();
                    divingDown = true;

                    currentState = NoctilumeState::DIVE;
                    attackTimer = 0.0f;
                    proximityTimer = 0.0f;
                }
            }
            else {
                proximityTimer = 0.0f;
            }
        }
    }

    // Interpolación de posición
    float transitionSpeed = 5.0f; // Ajusta la velocidad de transición
    Vector2D targetPosition;

    switch (currentState)
    {
    case NoctilumeState::IDLE:
        IdleFlying(dt);
        targetPosition = position; // Mantener la posición actual
        break;
    case NoctilumeState::FLYING:
        Flying(dt);
        targetPosition = position; // Mantener la posición actual
        break;
    case NoctilumeState::DIVE:
        Dive(dt);
        targetPosition = diveTargetPos; // La posición objetivo es la del dive
        break;
    case NoctilumeState::DEAD:
        break;
    }

    // Interpolación suave hacia la posición objetivo
    position = Lerp(position, targetPosition, dt * transitionSpeed);

    if (pbody != nullptr) {
        pbody->body->SetTransform(
            b2Vec2(PIXEL_TO_METERS(position.x), PIXEL_TO_METERS(position.y)),
            0.0f
        );
    }

    return Enemy::Update(dt);
}

void Noctilume::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLAYER:
        break;
    case ColliderType::ATTACK:
        break;
    }
}

bool Noctilume::CleanUp()
{
    return false;
}

void Noctilume::IdleFlying(float dt) {
    timePassed += dt;

    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();
    Vector2D broodPos = (pbody != nullptr)
        ? Vector2D{ static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().x)), static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().y)) }
    : position;

    float horizontalRange = 500.0f;
    float waveAmplitude = 4.5f;
    float waveFrequency = 0.00035f;

    // Movimiento horizontal tipo "ping-pong" (más directo)
    float pingPong = fmod(timePassed * waveFrequency * 2 * horizontalRange, 2 * horizontalRange);
    broodPos.x = 1200 + ((pingPong < horizontalRange) ? pingPong : (2 * horizontalRange - pingPong));

    // Movimiento vertical con ondulación suave
    broodPos.y = 500 + sin(timePassed * waveFrequency+0.5) * waveAmplitude;

    position = broodPos;

    if (pbody != nullptr) {
        pbody->body->SetTransform(
            b2Vec2(PIXEL_TO_METERS(position.x), PIXEL_TO_METERS(position.y)),
            0.0f
        );
    }
}
void Noctilume::Flying(float dt) {
    timePassed += dt;

    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();
    Vector2D broodPos = (pbody != nullptr)
        ? Vector2D{ static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().x)), static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().y)) }
    : position;

    // ==== Movimiento vertical hacia una altura fija sobre el jugador ====
    float hoverHeight = 300.0f;
    float targetY = playerPos.y - hoverHeight;
    float speedY = 0.1f;
    float deltaY = targetY - broodPos.y;

    broodPos.y += (deltaY > 0 ? speedY : -speedY) * dt;

    if (fabs(deltaY) < speedY * dt) {
        broodPos.y = targetY;
    }

    // ==== Movimiento horizontal con retardo usando lerping ====
    static float delayedPlayerX = playerPos.x; // valor suavizado persistente
    float smoothingFactor = 0.02f; // cuanto menor, más retardo

    // Actualizamos el valor suavizado
    delayedPlayerX += (playerPos.x - delayedPlayerX) * smoothingFactor;

    float oscillationAmplitude = 200.0f;
    float oscillationSpeed = 0.002f;

    broodPos.x = delayedPlayerX + oscillationAmplitude * sin(oscillationSpeed * timePassed + 2);

    // ==== Asignar nueva posición ====
    position = broodPos;

    if (pbody != nullptr) {
        pbody->body->SetTransform(
            b2Vec2(PIXEL_TO_METERS(position.x), PIXEL_TO_METERS(position.y)),
            0.0f
        );
    }
}

void Noctilume::Dive(float dt)
{
    timePassed += dt;

    static const float diveDuration = 1500.0f;
    diveElapsedTime += dt;

    float t = diveElapsedTime / diveDuration;
    if (t > 1.0f) {
        // Guardar la última posición para la transición suave (opcional)
        position = diveTargetPos; // Asegurarse que termine exactamente aquí
        currentState = NoctilumeState::FLYING;
        diveElapsedTime = 0.0f;
        return;
    }

    // Interpolación cuadrática
    Vector2D p0 = diveStartPos;
    Vector2D p1 = diveTargetPos;
    Vector2D p2 = {
        2 * diveTargetPos.x - diveStartPos.x,
        diveStartPos.y + 200 // más caída
    };

    float u = 1.0f - t;
    Vector2D newPos = p0 * (u * u) + p1 * (2 * u * t) + p2 * (t * t);
    position = newPos;

    // Mover el cuerpo
    if (pbody != nullptr) {
        pbody->body->SetTransform(
            b2Vec2(PIXEL_TO_METERS(position.x), PIXEL_TO_METERS(position.y)),
            0.0f
        );
    }

}

float Noctilume::DistanceToPlayer()
{
    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();

    float deltaX = playerPos.x - position.x;
    float deltaY = playerPos.y - position.y;

    return sqrt(deltaX * deltaX + deltaY * deltaY);
}
