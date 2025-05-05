#include "Hypnoviper.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"


Hypnoviper::Hypnoviper() : Enemy(EntityType::HYPNOVIPER) {
}

Hypnoviper::~Hypnoviper() {
}

bool Hypnoviper::Awake() {
    return Enemy::Awake();
}

bool Hypnoviper::Start() {
    //Add a physics to an item - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX(), (int)position.getY(), texW / 1.3, texH / 1.2, bodyType::DYNAMIC, 130, 20);

    //Assign collider type
    pbody->ctype = ColliderType::PLATFORM;

    pbody->listener = this;

    // Initialize pathfinding
    pathfinding = new Pathfinding();
    ResetPath();

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node enemyNode = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
    {
        if (std::string(enemyNode.attribute("type").as_string()) == type)
        {
            texture = Engine::GetInstance().textures.get()->Load(enemyNode.attribute("texture").as_string());
            sleepAnim.LoadAnimations(enemyNode.child("sleep"));
            hitAnim.LoadAnimations(enemyNode.child("hit"));
            deadAnim.LoadAnimations(enemyNode.child("dead"));
        }
    }

    b2Fixture* fixture = pbody->body->GetFixtureList();
    if (fixture) {
        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_PLAYER | CATEGORY_ATTACK;
        fixture->SetFilterData(filter);
    }

    currentAnimation = &sleepAnim;

    return true;
}

bool Hypnoviper::Update(float dt) {

    switch (currentState)
    {
    case HypnoviperState::SLEEPING:
        if (currentAnimation != &sleepAnim) currentAnimation = &sleepAnim;

        break;
    case HypnoviperState::HITTED:
        if (currentAnimation != &hitAnim) currentAnimation = &hitAnim;

        if (hitTimer.ReadMSec() > 500)
        {
            currentState = HypnoviperState::DEAD;
        }

        break;
    case HypnoviperState::DEAD:
        if (currentAnimation != &deadAnim) currentAnimation = &deadAnim;
        
        pbody->body->GetFixtureList()->SetSensor(true);
        pbody->body->SetGravityScale(0);
        break;
    }

    pbody->body->SetLinearVelocity(b2Vec2_zero);
    pbody->body->SetAngularVelocity(0);

    return Enemy::Update(dt);
}

bool Hypnoviper::PostUpdate() {
    Enemy::PostUpdate();
    if (currentState == HypnoviperState::DEAD && currentAnimation->HasFinished()) {
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }

    return true;
}

bool Hypnoviper::CleanUp() {
    return Enemy::CleanUp();
}


void Hypnoviper::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
        if (currentState == HypnoviperState::SLEEPING) {
            currentState = HypnoviperState::HITTED;
            hitTimer.Start();
        }
        break;
    }
}