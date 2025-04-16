#include "Scurver.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"


Scurver::Scurver() : Enemy(EntityType::SCURVER) {
}

Scurver::~Scurver() {
}

bool Scurver::Awake() {
    return Enemy::Awake();
}

bool Scurver::Start() {
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

    currentAnimation = &idleAnim;

    return Enemy::Start();
}

bool Scurver::Update(float dt) {
    if (pathfinding->HasFoundPlayer()) {
        if (currentState == ScurverState::IDLE) {
            currentState = ScurverState::ATTACK;
        }
    }

    switch (currentState)
    {
    case ScurverState::IDLE:
        if (currentAnimation != &idleAnim) currentAnimation = &idleAnim;
        break;
    case ScurverState::ATTACK:
        if (currentAnimation != &attackAnim) currentAnimation = &attackAnim;
        Attack(dt);
        break;
    case ScurverState::SLIDE:
        if (currentAnimation != &slideAnim) currentAnimation = &slideAnim;
        Slide(dt);
        break;
    case ScurverState::DEAD:
        if (currentAnimation != &deadAnim) currentAnimation = &deadAnim;
        break;
    }

    return Enemy::Update(dt);
}


bool Scurver::PostUpdate() {
    
    if (currentState == ScurverState::DEAD && currentAnimation->HasFinished()) {
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }

    return true;
}

bool Scurver::CleanUp() {
    return Enemy::CleanUp();
}

void Scurver::Attack(float dt) {
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
        currentState = ScurverState::SLIDE;
        timer.Start();

    }

    float exponentialVelocityIncrease = velocityBase * (pow(exponentialFactor, dt));

    currentVelocity.x += direction * exponentialVelocityIncrease;

    currentVelocity.x = fmin(fmax(currentVelocity.x, -maxSpeed), maxSpeed);

    pbody->body->SetLinearVelocity(currentVelocity);

    previousDirection = direction;
}

void Scurver::Slide(float dt) {
    b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();

    float frictionFactor = 0.97f;

    if (fabs(currentVelocity.x) > 0.3f) {
        currentVelocity.x *= frictionFactor;
    }
    else {
        currentVelocity.x = 0.0f;
        currentState = ScurverState::IDLE;
    }

    pbody->body->SetLinearVelocity(currentVelocity);
}

void Scurver::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
        currentState = ScurverState::DEAD;
        break;
    }
}