#include "Bloodrusher.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"


Bloodrusher::Bloodrusher() : Enemy(EntityType::BLOODRUSHER) {
}

Bloodrusher::~Bloodrusher() {
}

bool Bloodrusher::Awake() {
    return Enemy::Awake();
}

bool Bloodrusher::Start() {
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node enemyNode = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
    {
        if (std::string(enemyNode.attribute("type").as_string()) == type)
        {
            texture = Engine::GetInstance().textures.get()->Load(enemyNode.attribute("texture").as_string());
            idleAnim.LoadAnimations(enemyNode.child("idle"));
            attackAnim.LoadAnimations(enemyNode.child("attack"));
            slideAnim.LoadAnimations(enemyNode.child("slide"));
        }
    }

    currentAnimation = &idleAnim;

    return Enemy::Start();
}

bool Bloodrusher::Update(float dt) {
    if (pathfinding->HasFoundPlayer()) {
        if (currentState == BloodrusherState::IDLE) {
            currentState = BloodrusherState::ATTACKING;
        }
    }
    GetPosition();
    switch (currentState)
    {
    case BloodrusherState::IDLE:
        Idle();
        break;
    case BloodrusherState::ATTACKING:
        Attack(dt);
        break;
    case BloodrusherState::SLIDING:
        Slide(dt);
        break;
    case BloodrusherState::DEAD:
        break;
    }

    return Enemy::Update(dt);
}


bool Bloodrusher::CleanUp() {
    return Enemy::CleanUp();
}

void Bloodrusher::Idle() {
    currentAnimation = &idleAnim;
}

void Bloodrusher::Attack(float dt)
{
    currentAnimation = &attackAnim;

    b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();

    static float exponentialFactor = 1.007f;
    static float velocityBase = 0.15f;
    static float maxSpeed = 15.0f;

    Vector2D nextTile = pathfinding->pathTiles.front();
    Vector2D nextTileWorld = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

    float direction = (nextTileWorld.getX() > position.getX()) ? 1.0f :
        (nextTileWorld.getX() < position.getX() ? -1.0f : 0.0f);

    if (direction != previousDirection)
    {
        currentState = BloodrusherState::SLIDING;
        timer.Start();

    }

    float exponentialVelocityIncrease = velocityBase * (pow(exponentialFactor, dt));

    currentVelocity.x += direction * exponentialVelocityIncrease;

    currentVelocity.x = fmin(fmax(currentVelocity.x, -maxSpeed), maxSpeed);

    pbody->body->SetLinearVelocity(currentVelocity);

    previousDirection = direction;
}

void Bloodrusher::Slide(float dt) {
    currentAnimation = &slideAnim;

    b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();

    float frictionFactor = 0.97f;

    if (fabs(currentVelocity.x) > 0.3f) {
        currentVelocity.x *= frictionFactor;
    }
    else {
        currentVelocity.x = 0.0f;
        currentState = BloodrusherState::IDLE;
    }

    // Aplicar la nueva velocidad al cuerpo
    pbody->body->SetLinearVelocity(currentVelocity);
}




void Bloodrusher::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::WALL:
        if (currentState == BloodrusherState::ATTACKING)
        {
            currentState = BloodrusherState::DEAD;
            Engine::GetInstance().entityManager->DestroyEntity(this);
        }
        break;
    case ColliderType::ATTACK:
        currentState = BloodrusherState::DEAD;
        break;
    }
}