#include "DungBeetle.h"
#include "Module.h"
#include "PressureSystemController.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "PressureSystemController.h"
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
            ballAnim.LoadAnimations(node.child("ball"));
            deathAnim.LoadAnimations(node.child("death"));

            currentAnimation = &idleAnim;
            break;
        }
    }

    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texW / 2, (int)position.getY() + texH / 1.5, texW / 1.5, texH / 1.5, bodyType::STATIC);

    if (pbody && pbody->body) {
        b2Fixture* fixture = pbody->body->GetFixtureList();
        fixture->SetRestitution(1.0f);
        fixture->SetFriction(0.0f);
        fixture->SetDensity(1.0f);

        pbody->body->SetLinearDamping(0.0f);
        pbody->body->SetAngularDamping(0.0f);
        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_WALL | CATEGORY_PLAYER | CATEGORY_PLATFORM | CATEGORY_ATTACK;
        fixture->SetFilterData(filter);
        pbody->ctype = ColliderType::ENEMY;
        pbody->listener = this;
    }

    maxSteps = 15;
    firstBallTimer = 0.0f;
    firstBallLaunched = false;

    return Enemy::Start();
}

bool DungBeetle::Update(float dt) {

    playerPos = Engine::GetInstance().scene->GetPlayerPosition();
    if (!firstBallLaunched) {
        firstBallTimer += dt;
        if (firstBallTimer >= 3000.0f) {
            currentState = DungBeetleState::ANGRY;
            firstBallLaunched = true;
            ballsThrown = 0;
            lastPuzzleState = -1;
            currentStatePuzzle = 0;
        }
    }

    CheckState(dt);
    CollisionNavigationLayer();
    return Enemy::Update(dt);
}
bool DungBeetle::PostUpdate() {
    if (currentState == DungBeetleState::HIT && currentAnimation == &deathAnim && currentAnimation->HasFinished()) {
            pbody->body->GetFixtureList()->SetSensor(true);
        for (auto ball : dungBalls) {
            if (ball->currentAnimation != &ball->destroyAnim) {
                ball->currentAnimation = &ball->destroyAnim;
                ball->destroyAnim.Reset();
            }
        }

        Engine::GetInstance().pressureSystem->OpenDoor(2);
        Engine::GetInstance().entityManager->DestroyEntity(this);
    }

    return true;
}

bool DungBeetle::CleanUp() {
    Engine::GetInstance().physics->DeletePhysBody(pbody);
    return true;
}
void DungBeetle::OnCollision(PhysBody* physA, PhysBody* physB) {

    switch (physB->ctype) {
    case ColliderType::ATTACK:
        if (currentState == DungBeetleState::BALLMODE)
            currentState = DungBeetleState::HIT;
        break;
    case ColliderType::PLAYER:
        if (currentState == DungBeetleState::BALLMODE) {
            Bounce();
        }
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
    case DungBeetleState::HIT: Hit(); break;
    }
}
void DungBeetle::Idle()
{
    currentAnimation = &idleAnim;
    angryAnim.Reset();
    currentStatePuzzle = CheckPuzzleState();

    if (currentStatePuzzle != lastPuzzleState && ballsThrown <= 2) {
        currentState = DungBeetleState::ANGRY;
    }
    else if (ballsThrown < 2) {
        hasThrown = false;
    }
    else if (currentStatePuzzle == 2 && ballsThrown == 2 && !hasLaunched) {
        currentState = DungBeetleState::BALLMODE;
    }

    lastPuzzleState = currentStatePuzzle;
}


void DungBeetle::Angry()
{
    throwingAnim.Reset();
    currentAnimation = &angryAnim;

    if (currentAnimation->HasFinished())
    {
        if ((CheckPuzzleState() == 0 && ballsThrown == 0) ||
            (CheckPuzzleState() == 1 && ballsThrown == 1)) {
            currentState = DungBeetleState::THROW;
        }
        else if (CheckPuzzleState() == 2 && ballsThrown == 2 && !hasLaunched) {
            currentState = DungBeetleState::BALLMODE;
        }
        else {
            currentState = DungBeetleState::IDLE;
        }
    }
}

void DungBeetle::Throw(float dt)
{
    currentAnimation = &throwingAnim;
    if (currentAnimation->HasFinished())
    {
        if (!hasThrown && ballsThrown < 2) {
            b2Vec2 dir((direction < 0) ? -1.0f : 1.0f, 1.0f);

            float spawnX = METERS_TO_PIXELS(pbody->body->GetPosition().x);
            float spawnY = METERS_TO_PIXELS(pbody->body->GetPosition().y) + 50.0f;
            DungBeetleBall* ball = new DungBeetleBall(spawnX, spawnY, throwSpeed, dir);
            Engine::GetInstance().entityManager->AddEntity(ball);
            AssignBalls(ball);
            ballsThrown++;
            hasThrown = true;
        }

        lastPuzzleState = currentStatePuzzle;
        currentState = DungBeetleState::IDLE;
    }
}
void DungBeetle::BallMode()
{
    if (!isDynamic)
    {
        ChangeBodyType();
        currentAnimation = &ballModeAnim;
        time = 0.0f;
    }

    if (currentAnimation->HasFinished())
    {
        if (!hasLaunched)
        {
            currentAnimation = &ballAnim;

            b2Vec2 dir(direction < 0 ? 1.0f : -1.0f, 1.0f);
            pbody->body->SetLinearVelocity(ballModeSpeed * dir);

            hasLaunched = true;

            ChangeColliderRadius(70.0f, true);
        }
    }
}

void DungBeetle::Hit() {
    pbody->body->SetLinearVelocity(b2Vec2_zero);
    pbody->body->SetAngularVelocity(0);
    currentAnimation = &deathAnim;
}
void DungBeetle::ChangeBodyType() {
    pbody->body->SetType(b2_dynamicBody);
    isDynamic = true;
}
int DungBeetle::CheckPuzzleState() {

    int PuzzlesDone = Engine::GetInstance().pressureSystem.get()->GetActivePlatesCount(3);
    return PuzzlesDone;
}
void DungBeetle::Bounce()
{
    b2Vec2 velocity = pbody->body->GetLinearVelocity();
    if (velocity.LengthSquared() < 0.01f) return;

    velocity.Normalize();
    Vector2D mapPos = Engine::GetInstance().map->WorldToMap(
        METERS_TO_PIXELS(pbody->body->GetPosition().x),
        METERS_TO_PIXELS(pbody->body->GetPosition().y)
    );

    MapLayer* layer = Engine::GetInstance().map->GetNavigationLayer();
    b2Vec2 normal(0.0f, 0.0f);

    if (velocity.x < 0 && layer->Get(mapPos.x - 1, mapPos.y)) normal += b2Vec2(1, 0);
    else if (velocity.x > 0 && layer->Get(mapPos.x + 1, mapPos.y)) normal += b2Vec2(-1, 0);
    if (velocity.y < 0 && layer->Get(mapPos.x, mapPos.y - 1)) normal += b2Vec2(0, 1);
    else if (velocity.y > 0 && layer->Get(mapPos.x, mapPos.y + 1)) normal += b2Vec2(0, -1);

    b2Vec2 finalVel;

    if (normal.LengthSquared() > 0.0f)
    {
        normal.Normalize();
        b2Vec2 reflected = velocity - 2.0f * b2Dot(velocity, normal) * normal;

        float angle = atan2(reflected.y, reflected.x);
        angle += (((rand() % 100) / 100.0f - 0.5f) * 0.2f); // random ±0.1 radians
        finalVel.Set(cosf(angle), sinf(angle));
        finalVel *= ballModeSpeed;

        if (finalVel.LengthSquared() < 0.01f)
            finalVel = ballModeSpeed * normal;
    }
    else
    {
        finalVel = -ballModeSpeed * velocity;
    }

    pbody->body->SetLinearVelocity(finalVel);
    pbody->body->SetAngularVelocity(0.0f);
}
void DungBeetle::ChangeColliderRadius(float newRadius, bool isSensor)
{
    if (!pbody || !pbody->body) return;

    b2Fixture* oldFixture = pbody->body->GetFixtureList();
    if (oldFixture) {
        pbody->body->DestroyFixture(oldFixture);
    }

    b2CircleShape circleShape;
    circleShape.m_radius = PIXEL_TO_METERS(newRadius);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.0f;
    fixtureDef.restitution = 1.0f;

    fixtureDef.isSensor = isSensor;
    fixtureDef.filter.categoryBits = CATEGORY_ENEMY;
    fixtureDef.filter.maskBits = CATEGORY_WALL | CATEGORY_PLAYER | CATEGORY_PLATFORM | CATEGORY_ATTACK;

    pbody->body->CreateFixture(&fixtureDef);
}
void DungBeetle::CollisionNavigationLayer()
{
    b2Vec2 vel = pbody->body->GetLinearVelocity();
    if (vel.LengthSquared() < 0.01f) return;

    b2Vec2 pos = pbody->body->GetPosition();
    Vector2D projected = Engine::GetInstance().map->WorldToMap(
        METERS_TO_PIXELS(pos.x + vel.x),
        METERS_TO_PIXELS(pos.y + vel.y)
    );

    if (projected != currentTileMap)
    {
        if (Engine::GetInstance().map->GetNavigationLayer()->Get(projected.x, projected.y))
            Bounce();
        currentTileMap = projected;
    }
}

void DungBeetle::AssignBalls(DungBeetleBall* ball)
{
    dungBalls.push_back(ball);
}
