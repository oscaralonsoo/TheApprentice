#include "DungBeetle.h"
#include "Module.h"
#include "PressureSystemController.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Entity.h"
#include "Log.h"
#include <cmath>
#include "DungBeetleBall.h"

DungBeetle::DungBeetle() : Enemy(EntityType::DUNGBEETLE) {}

DungBeetle::~DungBeetle() {}

bool DungBeetle::Awake() {
    return true;
}

bool DungBeetle::Start() {
    pugi::xml_document configDoc;
    if (!configDoc.load_file("config.xml")) return false;

    const std::string typeName = "DungBeetle";
    for (auto node : configDoc.child("config").child("scene").child("animations").child("enemies").children("enemy")) {
        if (node.attribute("type").as_string() == typeName) {
            texture = Engine::GetInstance().textures->Load(node.attribute("texture").as_string());
            texH = node.attribute("h").as_int();
            texW = node.attribute("w").as_int();
            idleAnim.LoadAnimations(node.child("idle"));
            angryAnim.LoadAnimations(node.child("angry"));
            throwingAnim.LoadAnimations(node.child("throwing"));
            ballModeAnim.LoadAnimations(node.child("ballMode"));
            hitAnim.LoadAnimations(node.child("hit"));
            dieAnim.LoadAnimations(node.child("death"));
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
    if (pbody && pbody->body) {
        b2Fixture* fixture = pbody->body->GetFixtureList();
        fixture->SetRestitution(1.0f);
        fixture->SetFriction(0.0f);
        fixture->SetDensity(1.0f);

        pbody->body->SetLinearDamping(0.0f);
        pbody->body->SetAngularDamping(0.0f);
        pbody->body->SetBullet(true);
        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_WALL | CATEGORY_PLAYER | CATEGORY_PLATFORM | CATEGORY_ATTACK;
        fixture->SetFilterData(filter);
        pbody->ctype = ColliderType::ENEMY;
        pbody->listener = this;
    }

    maxSteps = 15;

    return Enemy::Start();
}

bool DungBeetle::Update(float dt) {
    playerPos = Engine::GetInstance().scene->GetPlayerPosition();

    CheckState(dt);

    return Enemy::Update(dt);
}

bool DungBeetle::PostUpdate() {
    if (currentState == DungBeetleState::DEAD && currentAnimation->HasFinished())
    {
        pbody->body->GetFixtureList()->SetSensor(true);
        Engine::GetInstance().entityManager->DestroyEntity(this);
    }
    return true;
}

bool DungBeetle::CleanUp() {
    return Enemy::CleanUp();
}
void DungBeetle::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::ATTACK:
        if (currentState == DungBeetleState::BALLMODE)
            currentState = DungBeetleState::HIT;
        break;
    case ColliderType::PLATFORM:
    case ColliderType::WALL: 
        if (currentState == DungBeetleState::BALLMODE) {
            Bounce();
        }
        if (currentState == DungBeetleState::HIT && currentAnimation->HasFinished())
            currentState = DungBeetleState::DEAD;
        break;
    case ColliderType::PLAYER:
        // TODO TONI --- DETECTAR COLISION!
        Bounce();
        break;
    }
}
void DungBeetle::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLAYER:
        break;
    }
}
void DungBeetle::CheckState(float dt)
{
    switch (currentState) {
    case DungBeetleState::IDLE: Idle(); break;
    case DungBeetleState::ANGRY: Angry();  break;
    case DungBeetleState::THROW: Throw(dt); break;
    case DungBeetleState::BALLMODE: BallMode();  break;
    case DungBeetleState::HIT: currentAnimation = &hitAnim; break;
    case DungBeetleState::DEAD:currentAnimation = &dieAnim; break;
    }
}

void DungBeetle::Idle()
{
    currentAnimation = &idleAnim;
    CheckPuzzleState();
    if (CheckPuzzleState() != 0)
    {
        currentState = DungBeetleState::ANGRY;
    }
}

void DungBeetle::Angry()
{
    currentAnimation = &angryAnim;
    //if (currentAnimation->HasFinished())
    //{
        if (CheckPuzzleState() == 1 || CheckPuzzleState() == 2)
            currentState = DungBeetleState::THROW;
        else
            currentState = DungBeetleState::BALLMODE;
    //}
}

void DungBeetle::Throw(float dt)
{
    currentAnimation = &throwingAnim;

    if (!hasThrown) {
        float angle = ((float)rand() / RAND_MAX) * (7.0f * M_PI / 4.0f - 5.0f * M_PI / 4.0f) + (5.0f * M_PI / 4.0f);
        b2Vec2 dir(cosf(angle), sinf(angle));
        dir.Normalize();

        float spawnX = METERS_TO_PIXELS(pbody->body->GetPosition().x);
        float spawnY = METERS_TO_PIXELS(pbody->body->GetPosition().y);

        Engine::GetInstance().entityManager->AddEntity(new DungBeetleBall(spawnX, spawnY, throwSpeed, dir));
        hasThrown = true;
    }

    if (currentAnimation->HasFinished())
    {
        currentState = DungBeetleState::IDLE;
        hasThrown = false;
    }
}


void DungBeetle::BallMode()
{
    if (!isDynamic)
        ChangeBodyType();

    currentAnimation = &ballModeAnim;

    if (!hasLaunched)
    {
        float angle = ((float)rand() / RAND_MAX) * (7.0f * M_PI / 4.0f - 5.0f * M_PI / 4.0f) + (5.0f * M_PI / 4.0f);
        b2Vec2 dir(cosf(angle), sinf(angle));
        dir.Normalize();

        pbody->body->SetLinearVelocity(ballModeSpeed * dir);

        hasLaunched = true;
    }
}

void DungBeetle::ChangeBodyType() {
        pbody->body->SetType(b2_dynamicBody);
        isDynamic = true;
}
int DungBeetle::CheckPuzzleState() {
    int PuzzlesDone = 3; // TODO OSCAR --- CURRENT PRESSED PLATES
    return PuzzlesDone;

}
void DungBeetle::Bounce()
{
    b2Vec2 velocity = pbody->body->GetLinearVelocity();

    if (velocity.Length() > 0) {
        // Normalizar dirección actual
        velocity.Normalize();
        // Ángulo actual de movimiento
        float currentAngle = atan2f(velocity.y, velocity.x);

        float maxOffset = 50.0f * (M_PI / 180.0f); 
        float randomOffset = ((float)rand() / RAND_MAX) * (2.0f * maxOffset) - maxOffset;

        // Nuevo ángulo con desviación
        float newAngle = currentAngle + randomOffset;

        // Nueva dirección
        b2Vec2 newDir(cosf(newAngle), sinf(newAngle));
        newDir.Normalize();

        float speedVariation = ballModeSpeed * 1.05f;

        // Establecer nueva velocidad
        pbody->body->SetLinearVelocity(speedVariation * newDir);
    }

    // Detener rotación
    pbody->body->SetAngularVelocity(0.0f);
}




