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
        Attack();
        break;
    case NoctilumeState::CRASH:
        Crash();
        break;
    case NoctilumeState::DEAD:
        Die();
        break;
    }

    pbody->body->SetTransform(b2Vec2(position.getX() / PIXELS_PER_METER, position.getY() / PIXELS_PER_METER), 0);

    return Enemy::Update(dt);
}
bool Noctilume::PostUpdate() {
    Enemy::PostUpdate();

    if (currentState == NoctilumeState::DEAD && currentAnimation && currentAnimation->HasFinished())
        Engine::GetInstance().entityManager->DestroyEntity(this);

    return true;
}
void Noctilume::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        currentState = NoctilumeState::CRASH;
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

    // TODO TONI -- Hacer que siga mas smooth
    delayedPlayerX += (playerPos.getX() - delayedPlayerX) ;
    delayedPlayerY += (playerPos.getY() - delayedPlayerY) ;

    const float hoverHeight = 250.0f;
    position.setY(position.getY() + (delayedPlayerY - hoverHeight - position.getY()) * 0.5f);

    const float oscillationAmplitude = 200.0f;
    const float oscillationSpeed = 0.002f;

    float sinValue = std::sin(oscillationSpeed * timePassed + 2.0f);
    position.setX(delayedPlayerX + oscillationAmplitude * sinValue);

    lastSinValue = sinValue;
}

void Noctilume::Attack( ) {
    currentAnimation = &attackAnim;

}
void Noctilume::Crash() {
    currentAnimation = &crashAnim;
    pbody->body->SetLinearVelocity(b2Vec2_zero);
    pbody->body->SetAngularVelocity(0);

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