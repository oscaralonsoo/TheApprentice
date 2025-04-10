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

    currentAnimation = &sleepAnim;

    return Enemy::Start();
}

bool Hypnoviper::Update(float dt) {

    switch (currentState)
    {
    case HypnoviperState::SLEEPING:
        if (currentAnimation != &sleepAnim) currentAnimation = &sleepAnim;

        break;
    case HypnoviperState::HITTED:
        if (currentAnimation != &hitAnim) currentAnimation = &hitAnim;
        if (hitTimer.ReadMSec() == 0) hitTimer.Start();

        if (hitTimer.ReadMSec() > 2000)
        {
            currentState = HypnoviperState::DEAD;
        }

        break;
    case HypnoviperState::DEAD:
        if (currentAnimation != &deadAnim) currentAnimation = &deadAnim;

        if (currentAnimation->HasFinished())
            Engine::GetInstance().entityManager.get()->DestroyEntity(this);

        break;
    }

    return Enemy::Update(dt);
}

bool Hypnoviper::PostUpdate() {
    
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
        currentState = HypnoviperState::HITTED;
        break;
    }
}