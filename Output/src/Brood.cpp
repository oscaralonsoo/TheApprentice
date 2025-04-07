#include "Brood.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Log.h"
#include <cmath>

Brood::Brood() : Enemy(EntityType::BROOD)
{
    waveOffset = static_cast<float>(rand() % 628) / 100.0f;
}

Brood::~Brood() {
}

bool Brood::Awake() {
    return Enemy::Awake();
}

bool Brood::Start() {
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    std::string type = "Brood";

    for (pugi::xml_node enemyNode = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy");
        enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {

        if (std::string(enemyNode.attribute("type").as_string()) == type) {
            texture = Engine::GetInstance().textures.get()->Load(enemyNode.attribute("texture").as_string());

            idleAnim.LoadAnimations(enemyNode.child("idle"));

            currentAnimation = &idleAnim;
            break;
        }
    }

    return Enemy::Start();
}

bool Brood::Update(float dt) {
    if (pathfinding->HasFoundPlayer()) {
        if (currentState == BroodState::IDLE) {
            currentState = BroodState::CHASING;
        }
    }
    switch (currentState)
    {
    case BroodState::IDLE:
        Idle(dt);
        break;
    case BroodState::CHASING:
        Chase(dt);
        break;
    case BroodState::DEAD:
        break;
    }
    return Enemy::Update(dt);
}
bool Brood::CleanUp() {
    return Enemy::CleanUp();
}

void Brood::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLAYER:
        break;
    case ColliderType::ATTACK:
        if (parent) {
            parent->OnBroodDeath(this);
        }
        Engine::GetInstance().entityManager->QueueEntityForDestruction(this);
        break;
    }
}

void Brood::Idle(float dt) {

}
void Brood::Chase(float dt) {
    timePassed += dt;

    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();
    Vector2D broodPos = (pbody != nullptr)
        ? Vector2D{ static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().x)), static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().y)) }
    : position;

    // Calcular dirección deseada
    Vector2D desiredDirection = playerPos - broodPos;
    float length = sqrt(desiredDirection.x * desiredDirection.x + desiredDirection.y * desiredDirection.y);
    if (length != 0) {
        desiredDirection.x /= length;
        desiredDirection.y /= length;
    }

    // Suavizar dirección
    float steeringSmoothness = 0.041f;
    direction.x = (1.0f - steeringSmoothness) * direction.x + steeringSmoothness * desiredDirection.x;
    direction.y = (1.0f - steeringSmoothness) * direction.y + steeringSmoothness * desiredDirection.y;

    float dirLen = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (dirLen != 0) {
        direction.x /= dirLen;
        direction.y /= dirLen;
    }

    // Movimiento base
    float speed = 0.4f;
    broodPos.x += direction.x * speed * dt;
    broodPos.y += direction.y * speed * dt;

    // Ondulación estilo espíritu
    float waveAmplitude = 4.0f;
    float waveFrequency = 0.005f;

    Vector2D perp = { -direction.y, direction.x };
    float wave = sin(timePassed * waveFrequency + waveOffset) * waveAmplitude;
    broodPos.x += perp.x * wave;
    broodPos.y += perp.y * wave;

    // Actualizar posición
    position = broodPos;
    if (pbody != nullptr) {
        pbody->body->SetTransform(
            b2Vec2(PIXEL_TO_METERS(position.x), PIXEL_TO_METERS(position.y)),
            0.0f
        );
    }
}

