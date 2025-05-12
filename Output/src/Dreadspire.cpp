#include "Dreadspire.h"
#include "DreadspireBullet.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Entity.h"
#include "Log.h"
#include <cmath>

Dreadspire::Dreadspire() : Enemy(EntityType::NOCTILUME) {}

Dreadspire::~Dreadspire() {}

bool Dreadspire::Awake() {
    return true;
}

bool Dreadspire::Start() {
    pugi::xml_document configDoc;
    if (!configDoc.load_file("config.xml")) return false;

    const std::string typeName = "Dreadspire";
    for (auto node : configDoc.child("config").child("scene").child("animations").child("enemies").children("enemy")) {
        if (node.attribute("type").as_string() == typeName) {
            texture = Engine::GetInstance().textures->Load(node.attribute("texture").as_string());
            idleAnim.LoadAnimations(node.child("idle"));
            dieAnim.LoadAnimations(node.child("die"));
            currentAnimation = &idleAnim;
            break;
        }
    }

    pbody = Engine::GetInstance().physics->CreateCircle(
        static_cast<int>(position.x + texH / 2),
        static_cast<int>(position.y + texH / 2),
        texH / 2,
        bodyType::STATIC
    );

    maxSteps = 15;

    return Enemy::Start();
}

bool Dreadspire::Update(float dt) {
    CheckState();

    playerPos = Engine::GetInstance().scene->GetPlayerPosition();

    if (playerPos.x < position.x) direction = -1;
    else direction = 1;

    switch (currentState) {
    case DreadspireState::IDLE: Idle(dt); break;
    case DreadspireState::RECHARGING: Recharge(dt); break;
    case DreadspireState::SHOOTING: Shoot(dt); break;
    case DreadspireState::DEAD:currentAnimation = &dieAnim;  break;
    }

    return Enemy::Update(dt);
}

bool Dreadspire::PostUpdate() {
    if (currentState == DreadspireState::DEAD)
        Engine::GetInstance().entityManager->DestroyEntity(this);
    return true;
}

bool Dreadspire::CleanUp() {
    return Enemy::CleanUp();
}

void Dreadspire::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::ATTACK:
            currentState = DreadspireState::DEAD;
        break;
    }
}

void Dreadspire::Idle(float dt) {
    currentAnimation = &idleAnim;
}

void Dreadspire::Shoot(float dt)
{
    currentAnimation = &shootingAnim;

    static float fireCooldown = 0;
    fireCooldown -= dt;

    if (fireCooldown > 0) return;

    b2Vec2 toPlayer = b2Vec2(playerPos.x - position.x, playerPos.y - position.y);
    float baseAngle = atan2(toPlayer.y, toPlayer.x);

    float angles[3] = { -M_PI / 4, 0.0f, M_PI / 4 };

    for (int i = 0; i < 3; ++i) {
        float shootingAngle = baseAngle + angles[i];
        b2Vec2 dir = b2Vec2(cos(shootingAngle), sin(shootingAngle));

        auto bullet = new DreadspireBullet(position.getX(), position.getY(), 6.0f, dir);
        Engine::GetInstance().entityManager->AddEntity(bullet);
    }

    fireCooldown = 2500.0f;

    if (currentAnimation->HasFinished())
    {
        currentState = DreadspireState::RECHARGING;
    }
}

void Dreadspire::Recharge(float dt)
{
    currentAnimation = &idleAnim;
}

void Dreadspire::CheckState() {

    if (pathfinding->HasFoundPlayer()) {
        currentState = DreadspireState::SHOOTING;
    }
    else {
        if (currentState != DreadspireState::IDLE) {
            currentState = DreadspireState::IDLE;
        }
    }
}

