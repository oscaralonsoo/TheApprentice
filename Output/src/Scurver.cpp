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
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 3, texH / 3, bodyType::DYNAMIC);

    //Assign collider type
    pbody->ctype = ColliderType::ENEMY;

    pbody->listener = this;

    // Initialize pathfinding
    pathfinding = new Pathfinding();
    ResetPath();

    b2Fixture* fixture = pbody->body->GetFixtureList();
    if (fixture) {
        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_ATTACK | CATEGORY_PLAYER_DAMAGE;
        fixture->SetFilterData(filter);
    }

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node enemyNode = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
    {
        if (std::string(enemyNode.attribute("type").as_string()) == type)
        {
            texture = Engine::GetInstance().textures.get()->Load(enemyNode.attribute("texture").as_string());
            attackAnim.LoadAnimations(enemyNode.child("idle"));
            deadAnim.LoadAnimations(enemyNode.child("dead"));
        }
    }

    currentAnimation = &attackAnim;

    return true;
}

bool Scurver::Update(float dt) {
    if (pathfinding->HasFoundPlayer()) {
        if (currentState == ScurverState::IDLE) {
            currentState = ScurverState::ATTACK;
        }
    }
    else {
        if (currentState == ScurverState::ATTACK || currentState == ScurverState::SLIDE) {
            currentState = ScurverState::IDLE;
        }
    }

    switch (currentState)
    {
    case ScurverState::IDLE:
        if (currentAnimation != &attackAnim) currentAnimation = &attackAnim;
        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);

        currentAnimation->SetPaused(true);
        break;
    case ScurverState::ATTACK:
        if (currentAnimation != &attackAnim) currentAnimation = &attackAnim;
        currentAnimation->SetPaused(false);

        Attack(dt);
        break;
    case ScurverState::SLIDE:
        if (currentAnimation != &attackAnim) currentAnimation = &attackAnim;
        Slide(dt);
        break;
    case ScurverState::DEAD:
        if (currentAnimation != &deadAnim) currentAnimation = &deadAnim;

        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);
        break;
    }

    return Enemy::Update(dt);
}


bool Scurver::PostUpdate() {
    Enemy::PostUpdate();
    if (currentState == ScurverState::DEAD && currentAnimation->HasFinished()) {
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }

    return true;
}

bool Scurver::CleanUp() {
    return Enemy::CleanUp();
}

void Scurver::Attack(float dt) {
    if (pathfinding->pathTiles.empty()) {
       
        pbody->body->SetLinearVelocity(b2Vec2(0, 0));
        return;
    }

    b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();

    static float exponentialFactor = 1.007f;
    static float velocityBase = 0.13f;
    static float maxSpeed = 10.0f;

    Vector2D nextTile = pathfinding->pathTiles.front();
    Vector2D nextTileWorld = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

    direction = (nextTileWorld.getX() > position.getX()) ? 1.0f :
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