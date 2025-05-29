#include "Bloodrusher.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Audio.h"


Bloodrusher::Bloodrusher() : Enemy(EntityType::BLOODRUSHER) {
}

Bloodrusher::~Bloodrusher() {
}

bool Bloodrusher::Awake() {
    return Enemy::Awake();
}

bool Bloodrusher::Start() {
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texW / 2, (int)position.getY() + texH / 2, texW / 1.5, texH / 2, bodyType::DYNAMIC, 0, 32);

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

    maxSteps = 15;

    currentAnimation = &idleAnim;

    soundMoveId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/bloodrusher_walk.ogg", 1.0f);
    soundSlideId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/bloodrusher_slide.ogg", 1.0f);
    soundDeadId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/monster_dead.ogg", 1.0f);

    return true;
}

bool Bloodrusher::Update(float dt) {
    if (!pathfinding->HasFoundPlayer()) {
        currentState = BloodrusherState::IDLE;
    }
    switch (currentState)
    {
    case BloodrusherState::IDLE:
        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);

        Idle();
        break;
    case BloodrusherState::ATTACKING:
        if (currentAnimation != &attackAnim) currentAnimation = &attackAnim;
        Attack(dt);
        break;
    case BloodrusherState::SLIDING:
        if (currentAnimation != &slideAnim) currentAnimation = &slideAnim;
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
    if (pathfinding->HasFoundPlayer()) {
        Vector2D nextTile = pathfinding->pathTiles.front();
        Vector2D nextTileWorld = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

        direction = (nextTileWorld.getX() > position.getX()) ? 1.0f :
            (nextTileWorld.getX() < position.getX() ? -1.0f : direction);

        if (IsGroundAhead()) {
            currentState = BloodrusherState::ATTACKING;
        }
        else {
            currentAnimation = &idleAnim;
        }
    }
    else {
        currentAnimation = &idleAnim;
    }
    moveSoundPlayed = false;
    slideSoundPlayed = false;
    deadSoundPlayed = false;
}

void Bloodrusher::Attack(float dt) {
    if (!moveSoundPlayed) {
        Engine::GetInstance().audio->PlayFx(soundMoveId, 1.0f, 0);
        moveSoundPlayed = true;
    }
    if (pathfinding->pathTiles.empty()) {
        pbody->body->SetLinearVelocity(b2Vec2(0, 0));
        return;
    }

    Vector2D nextTile = pathfinding->pathTiles.front();
    Vector2D nextTileWorld = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

    direction = (nextTileWorld.getX() > position.getX()) ? 1.0f :
        (nextTileWorld.getX() < position.getX() ? -1.0f : 0.0f);

    b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();

    static float exponentialFactor = 1.007f;
    static float velocityBase = 0.15f;
    static float maxSpeed = 15.0f;

    if (direction != previousDirection) currentState = BloodrusherState::SLIDING;

    float exponentialVelocityIncrease = velocityBase * (pow(exponentialFactor, dt));
    currentVelocity.x += direction * exponentialVelocityIncrease;

    currentVelocity.x = fmin(fmax(currentVelocity.x, -maxSpeed), maxSpeed);
    pbody->body->SetLinearVelocity(currentVelocity);

    previousDirection = direction;
}

void Bloodrusher::Slide(float dt) {
    b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();

    if (!slideSoundPlayed) {
        Engine::GetInstance().audio->PlayFx(soundSlideId, 1.0f, 0);
        slideSoundPlayed = true;
    }

    if (IsGroundAhead()) {
        timer.Start();
    }

    if (timer.ReadSec() > 0.5f) {
        currentState = BloodrusherState::IDLE;
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
        currentState = BloodrusherState::IDLE;
    }

    pbody->body->SetLinearVelocity(currentVelocity);
}

void Bloodrusher::Dead() {
    if (currentAnimation != &deadAnim) currentAnimation = &deadAnim;

    pbody->body->SetLinearVelocity(b2Vec2_zero);
    pbody->body->SetAngularVelocity(0);

    if (!deadSoundPlayed) {
        Engine::GetInstance().audio->PlayFx(soundDeadId, 1.0f, 0);
        deadSoundPlayed = true;
    }

}

void Bloodrusher::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::DESTRUCTIBLE_WALL:
        currentState = BloodrusherState::DEAD;
        break;
    }
}

bool Bloodrusher::IsGroundAhead() {
    Vector2D posMap = Engine::GetInstance().map.get()->WorldToMap(position.getX() + texW / 2, position.getY() + texH / 2);
    MapLayer* layer = Engine::GetInstance().map.get()->GetNavigationLayer();

    int frontY = posMap.y + 2;
    int checkOffset = 3;

    if (currentState == BloodrusherState::SLIDING) {
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

