#include "Brood.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Log.h"

Brood::Brood() : Enemy(EntityType::BROOD)
{
}

Brood::~Brood() {
}

bool Brood::Awake() {
    return Enemy::Awake();
}

bool Brood::Start() {
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

    return Enemy::Start();
}

bool Brood::Update(float dt) {
    if (pathfinding->HasFoundPlayer()) {
        if (currentState == BroodState::IDLE) {
            currentState = BroodState::CHASING;
        }
    }
    switch (currentState)
    {
    case BroodState::IDLE:
        Idle(dt);
        break;
    case BroodState::CHASING:
        Chase(dt);
        break;
    case BroodState::RETURNING:
        ReturnToPlayer(dt);
        break;
    case BroodState::DEAD:
        break;
    }
    return Enemy::Update(dt);
}
bool Brood::CleanUp() {
    return Enemy::CleanUp();
}

void Brood::OnCollision(PhysBody* physA, PhysBody* physB) {
    
        if (physA->ctype == ColliderType::PLAYER)
        {
            currentState = BroodState::DEAD;
        }
}

void Brood::Idle(float dt) {

}

void Brood::Chase(float dt) {



}

void Brood::ReturnToPlayer(float dt) {

}
