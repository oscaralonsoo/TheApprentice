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
}
Noctilume::~Noctilume() {}
bool Noctilume::Awake()
{
    return true;
}
bool Noctilume::Start() {
    pugi::xml_document configDoc;
    if (!configDoc.load_file("config.xml")) return false;

    const std::string typeName = "Noctilume";
    for (auto node : configDoc.child("config").child("scene").child("animations").child("enemies").children("enemy")) {
        if (node.attribute("type").as_string() == typeName) {
            texture = Engine::GetInstance().textures->Load(node.attribute("texture").as_string());
            flyingAnim.LoadAnimations(node.child("idle"));
            attackAnim.LoadAnimations(node.child("attack"));
            crashAnim.LoadAnimations(node.child("crash"));
            dieAnim.LoadAnimations(node.child("die"));
            currentAnimation = &flyingAnim;
            break;
        }
    }

    pbody = Engine::GetInstance().physics->CreateCircleSensor( static_cast<int>(position.getX() + texH / 2),
        static_cast<int>(position.getY() + texH / 2),texH / 4,bodyType::DYNAMIC);

    originalPosition = position;

    maxSteps = 15;

    return Enemy::Start();
}
bool Noctilume::Update(float dt) {
    CheckState();

    switch (currentState)
    {
    case NoctilumeState::IDLE:
        Idle(dt);
        break;
    case NoctilumeState::CHASING:
        Chasing(dt);
        break;
    case NoctilumeState::ATTACK:
        Attack(dt);
        break;
    case NoctilumeState::CRASH:
        Crash(dt);
        break;
    case NoctilumeState::DEAD:
        Die();
        break;
    }

    pbody->body->SetTransform(b2Vec2(position.getX() / PIXELS_PER_METER, position.getY() / PIXELS_PER_METER), 0);

    return Enemy::Update(dt);
}
bool Noctilume::PostUpdate() {
    if (currentState == NoctilumeState::DEAD && currentAnimation && currentAnimation->HasFinished())
        Engine::GetInstance().entityManager->DestroyEntity(this);

    return true;
}
void Noctilume::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        currentState = NoctilumeState::CRASH;
        crashTimer = 0.0f;
        break;
    case ColliderType::ATTACK:
        if(currentState == NoctilumeState::CRASH)
            currentState = NoctilumeState::DEAD;
        break;
    }
}
bool Noctilume::CleanUp() {
    return Enemy::CleanUp();
}

void Noctilume::Idle(float dt) {
    currentAnimation = &flyingAnim;

    idleTime += dt;

    float xOffset = std::sin(idleTime * idleFrequency) * idleAmplitude;
    float newX = originalPosition.getX() + xOffset;

    float newY = originalPosition.getY();

    position.setX(newX);
    position.setY(newY);
}
void Noctilume::Chasing(float dt) {
    currentAnimation = &flyingAnim;

    timePassed += dt;

    const Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();

    delayedPlayerX += (playerPos.getX() - delayedPlayerX);
    delayedPlayerY += (playerPos.getY() - delayedPlayerY);

    position.setY(position.getY() + (delayedPlayerY - hoverHeight - position.getY()) * 0.5f);

    float sinValue = std::sin(oscillationSpeed * timePassed + 2.0f);
    position.setX(delayedPlayerX + oscillationAmplitude * sinValue);

    // Detección de cruce por el eje horizontal
    if ((lastSinValue < 0 && sinValue >= 0) || (lastSinValue > 0 && sinValue <= 0)) {
        oscillationCrosses++;
    }

    lastSinValue = sinValue;

    if (oscillationCrosses >= 6) {
        currentState = NoctilumeState::ATTACK;
        oscillationCrosses = 0;
    }
}
void Noctilume::Attack(float dt) {
    currentAnimation = &attackAnim;


}
void Noctilume::Crash(float dt) {
    currentAnimation = &crashAnim;

    pbody->body->SetLinearVelocity(b2Vec2_zero);
    pbody->body->SetAngularVelocity(0);

    crashTimer += dt;

    if (crashTimer >= 3000.0f) {
        if (pathfinding->HasFoundPlayer()) {
            currentState = NoctilumeState::CHASING;
        }
        else {
            currentState = NoctilumeState::IDLE;
        }
        crashTimer = 0.0f; // Por si vuelve a entrar en crash luego
    }
}
void Noctilume::Die() {
    currentAnimation = &dieAnim;
    if (pbody && pbody->body) {
        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);
        if (pbody->body->GetFixtureList())
            pbody->body->GetFixtureList()->SetSensor(true);
    }
}
void Noctilume::CheckState() {
    if (currentState == NoctilumeState::ATTACK || currentState == NoctilumeState::CRASH || currentState == NoctilumeState::DEAD)
        return;

    if (pathfinding->HasFoundPlayer()) {
        if (currentState != NoctilumeState::CHASING)
            currentState = NoctilumeState::CHASING;
    }
    else {
        if (currentState != NoctilumeState::IDLE)
            currentState = NoctilumeState::IDLE;
    }
}