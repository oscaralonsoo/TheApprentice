#include "Broodheart.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Log.h"

Broodheart::Broodheart() : Enemy(EntityType::BROODHEART) {
}

Broodheart::~Broodheart() {
}

bool Broodheart::Awake() {
    return Enemy::Awake();
}

bool Broodheart::Start() {
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    std::string type = "Broodheart";

    for (pugi::xml_node enemyNode = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy");
        enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {

        if (std::string(enemyNode.attribute("type").as_string()) == type) {
            texture = Engine::GetInstance().textures.get()->Load(enemyNode.attribute("texture").as_string());

            idleAnim.LoadAnimations(enemyNode.child("idle"));

            currentAnimation = &idleAnim;
            break;
        }
    }

    // En Broodheart::Start(), después de crear el pbody
    b2Fixture* fixture = pbody->body->GetFixtureList();
    if (fixture) {
        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_PLAYER_DAMAGE | CATEGORY_ATTACK;
        fixture->SetFilterData(filter);
    }


    return Enemy::Start();
}

bool Broodheart::Update(float dt) {
    switch (currentState)
    {
    case BroodheartState::IDLE:
        Idle(dt);
        break;
    case BroodheartState::SPAWN:
        Spawn(dt);
        break;
    case BroodheartState::DEAD:
        break;
    }
    return Enemy::Update(dt);
}

bool Broodheart::CleanUp() {
    return Enemy::CleanUp();
}

void Broodheart::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
        break;
    case ColliderType::PLAYER:
        // Damage the player
        break;
    case ColliderType::ATTACK:
        currentState = BroodheartState::DEAD;
        break;
    }
}

void Broodheart::Idle(float dt) {

}

void Broodheart::Spawn(float dt)
{
  
}

