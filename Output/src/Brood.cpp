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
  
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), texW / 2, bodyType::DYNAMIC);

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

    //Assign collider type
    pbody->ctype = ColliderType::ENEMY;

    pbody->listener = this;
    if (!gravity) pbody->body->SetGravityScale(0);
    // Initialize pathfinding
    pathfinding = new Pathfinding();
    ResetPath();

    b2Fixture* fixture = pbody->body->GetFixtureList();
    if (fixture) {
        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_ATTACK | CATEGORY_PLAYER_DAMAGE;
        fixture->SetFilterData(filter);
    }
 
    return true;
}

bool Brood::Update(float dt) {
    
    UpdateChaseState();

    switch (currentState)
    {
    case BroodState::IDLE:
        break;
    case BroodState::CHASING:
        Chase(dt);
        break;
    case BroodState::DEAD:
        break;
    }
    return Enemy::Update(dt);
}
bool Brood::PostUpdate(float dt) {
    return true;
}
bool Brood::CleanUp() {
    return Enemy::CleanUp();
}

void Brood::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLAYER:
        break;
    case ColliderType::ATTACK:
        if (broodHeart) {
            broodHeart->OnBroodDeath(this);
        }
        currentState = BroodState::DEAD; 
        break;
    }
}
void Brood::Chase(float dt) {
    timePassed += dt;

    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();
    Vector2D broodPos = (pbody != nullptr)
        ? Vector2D{ static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().x)), static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().y)) }
    : position;

    // Calculate pos
    Vector2D desiredDirection = playerPos - broodPos;
    float length = sqrt(desiredDirection.x * desiredDirection.x + desiredDirection.y * desiredDirection.y);
    if (length != 0) {
        desiredDirection.x /= length;
        desiredDirection.y /= length;
    }

    //Smooth redirection
    float steeringSmoothness = 0.030f;
    direction.x = (1.0f - steeringSmoothness) * direction.x + steeringSmoothness * desiredDirection.x;
    direction.y = (1.0f - steeringSmoothness) * direction.y + steeringSmoothness * desiredDirection.y;

    float dirLen = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (dirLen != 0) {
        direction.x /= dirLen;
        direction.y /= dirLen;
    }

    // Base Movement
    float speed = 0.25f;
    broodPos.x += direction.x * speed * dt;
    broodPos.y += direction.y * speed * dt;

    // Ondulation
    float waveAmplitude = 3.0;
    float waveFrequency = 0.003f;

    Vector2D perp = { -direction.y, direction.x };
    float wave = sin(timePassed * waveFrequency + waveOffset) * waveAmplitude;
    broodPos.x += perp.x * wave;
    broodPos.y += perp.y * wave;

    position = broodPos;
    if (pbody != nullptr) {
        pbody->body->SetTransform(
            b2Vec2(PIXEL_TO_METERS(position.x), PIXEL_TO_METERS(position.y)),
            0.0f
        );
    }
}
void Brood::UpdateChaseState()
{
    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();
    Vector2D broodPos = (pbody != nullptr)
        ? Vector2D{ static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().x)), static_cast<float>(METERS_TO_PIXELS(pbody->body->GetPosition().y)) }
    : position;
    distanceToPlayer = sqrt(pow(playerPos.x - broodPos.x, 2) + pow(playerPos.y - broodPos.y, 2));

    if (distanceToPlayer < detectionRange || pathfinding->HasFoundPlayer()) {
        currentState = BroodState::CHASING;
    }
}


