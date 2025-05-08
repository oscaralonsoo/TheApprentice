#include "Noctilume.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Entity.h"
#include "Log.h"
#include <cmath>

Noctilume::Noctilume() : Enemy(EntityType::NOCTILUME) {}

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

    pbody = Engine::GetInstance().physics->CreateCircle( static_cast<int>(position.getX() + texH / 2),
        static_cast<int>(position.getY() + texH / 2),texH / 4,bodyType::DYNAMIC);

    originalPosition = position;

    maxSteps = 10;

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
    timePassed += dt;

    float pingPong = fmod(timePassed * waveFrequency * 2 * horizontalRange, 2 * horizontalRange);
    float x = (pingPong < horizontalRange) ? pingPong : (2 * horizontalRange - pingPong);

    position.x = originalPosition.x + x;
    position.y = originalPosition.y + sin(timePassed * waveFrequency + waveOffset) * waveAmplitude;

}
void Noctilume::Chasing(float dt) {
    timePassed += dt;

    const Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();
    Vector2D noctPos = (pbody)
        ? Vector2D{ static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().x)),
                    static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().y)) }
    : position;

    delayedPlayerX += (playerPos.x - delayedPlayerX);
    delayedPlayerY += (playerPos.y - delayedPlayerY);

    noctPos.y += (delayedPlayerY - hoverHeight - noctPos.y) * 0.5f;
    
    float sinValue = sin(oscillationSpeed * timePassed + 2.0f);
    noctPos.x = delayedPlayerX + oscillationAmplitude * sinValue;

    // Set in case we go IDLE
    originalPosition = position;

    lastSinValue = sinValue;
    position = noctPos;
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