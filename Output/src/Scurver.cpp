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
    maxSteps = 15;

    return Enemy::Start();
}

bool Scurver::Update(float dt) {
    switch (currentState)
    {
    case ScurverState::IDLE:
        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);

        currentAnimation->SetPaused(true);

        if (pathfinding->HasFoundPlayer()) {
            Vector2D nextTile = pathfinding->pathTiles.front();
            Vector2D nextTileWorld = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

            direction = (nextTileWorld.getX() > position.getX()) ? 1.0f :
                (nextTileWorld.getX() < position.getX() ? -1.0f : direction);

            if (IsGroundAhead()) {
                currentState = ScurverState::ATTACK;
            }
        }

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

    Vector2D nextTile = pathfinding->pathTiles.front();
    Vector2D nextTileWorld = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

    direction = (nextTileWorld.getX() > position.getX()) ? 1.0f :
        (nextTileWorld.getX() < position.getX() ? -1.0f : 0.0f);

    if (!IsGroundAhead()) {
        currentState = ScurverState::SLIDE;
        return;
    }

    b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();

    static float exponentialFactor = 1.007f;
    static float velocityBase = 0.13f;
    static float maxSpeed = 10.0f;

    if (direction != previousDirection) currentState = ScurverState::SLIDE;

    float exponentialVelocityIncrease = velocityBase * (pow(exponentialFactor, dt));
    currentVelocity.x += direction * exponentialVelocityIncrease;

    currentVelocity.x = fmin(fmax(currentVelocity.x, -maxSpeed), maxSpeed);
    pbody->body->SetLinearVelocity(currentVelocity);

    previousDirection = direction;
}

void Scurver::Slide(float dt) {
    b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();

    if (IsGroundAhead()) {
        timer.Start();
    }

    if (timer.ReadSec() > 0.5f) {
        currentState = ScurverState::IDLE;
        currentVelocity.x = 0.0f;
        pbody->body->SetLinearVelocity(currentVelocity);
        return;
    }

    float frictionFactor = 0.92f;

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

bool Scurver::IsGroundAhead() {
    Vector2D posMap = Engine::GetInstance().map.get()->WorldToMap(position.getX() + texW / 2, position.getY() + texH / 2);
    MapLayer* layer = Engine::GetInstance().map.get()->GetNavigationLayer();

    int frontY = posMap.y + 1;
    int checkOffset = 3;

    if (currentState == ScurverState::SLIDE) {
        int frontLeftX = posMap.x - 1;
        int frontRightX = posMap.x + 1;

        bool groundLeft = layer->Get(frontLeftX, frontY);
        bool groundRight = layer->Get(frontRightX, frontY);

        return groundLeft && groundRight;
    }
    else {
        int frontX = posMap.x + direction * checkOffset;
        return layer->Get(frontX, frontY);
    }
}
