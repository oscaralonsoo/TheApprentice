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
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texW / 2, (int)position.getY() + texH / 2, texW / 1.5, texH / 2, bodyType::DYNAMIC, 0, 27);

    pbody->ctype = ColliderType::ENEMY;

    pbody->listener = this;

    if (!gravity) pbody->body->SetGravityScale(0);

    pathfinding = new Pathfinding();
    ResetPath();

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
            deadAnim.LoadAnimations(enemyNode.child("dead"));
        }
    }

    b2Fixture* fixture = pbody->body->GetFixtureList();
    if (fixture) {
        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_PLAYER_DAMAGE | CATEGORY_ATTACK;
        fixture->SetFilterData(filter);
    }

    currentAnimation = &idleAnim;

    return true;
}

bool Bloodrusher::Update(float dt) {
    if (pathfinding->HasFoundPlayer()) {
        if (currentState == BloodrusherState::IDLE) {
            currentState = BloodrusherState::ATTACKING;
        }
    }

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
        Dead();
        break;
    }

    return Enemy::Update(dt);
}

bool Bloodrusher::PostUpdate() {

    if (currentState == BloodrusherState::DEAD && currentAnimation->HasFinished()) {
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }

    return true;
}


bool Bloodrusher::CleanUp() {
    return Enemy::CleanUp();
}

void Bloodrusher::Idle() {
    if (currentAnimation != &idleAnim) currentAnimation = &idleAnim;
}

void Bloodrusher::Attack(float dt)
{
    if (currentAnimation != &attackAnim) currentAnimation = &attackAnim;

    if (pathfinding->pathTiles.empty()) {
        // No hay camino, as� que no hace nada o entra en un estado diferente
        pbody->body->SetLinearVelocity(b2Vec2(0, 0)); // detenerse o comportamiento alternativo
        return;
    }

    b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();

    Vector2D nextTile = pathfinding->pathTiles.front();
    Vector2D nextTileWorld = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

    direction = (nextTileWorld.getX() > position.getX()) ? 1.0f :
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
    if (currentAnimation != &slideAnim) currentAnimation = &slideAnim;

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


void Bloodrusher::Dead() {
    if (currentAnimation != &deadAnim) currentAnimation = &deadAnim;

    pbody->body->SetLinearVelocity(b2Vec2_zero);
    pbody->body->SetAngularVelocity(0);
}

void Bloodrusher::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::WALL:
        if (currentState == BloodrusherState::ATTACKING)
        {
            currentState = BloodrusherState::DEAD;
        }
        break;
    }
}