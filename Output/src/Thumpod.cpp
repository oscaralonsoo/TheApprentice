#include "Thumpod.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"


Thumpod::Thumpod() : Enemy(EntityType::THUMPOD) {
}

Thumpod::~Thumpod() {
}

bool Thumpod::Awake() {
    return Enemy::Awake();
}

bool Thumpod::Start() {
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node enemyNode = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
    {
        if (std::string(enemyNode.attribute("type").as_string()) == type)
        {
            texture = Engine::GetInstance().textures.get()->Load(enemyNode.attribute("texture").as_string());
            idleAnim.LoadAnimations(enemyNode.child("idle"));
        }
    }

    currentAnimation = &idleAnim;

    return Enemy::Start();
}

bool Thumpod::Update(float dt) {

    switch (currentState)
    {
    case ThumpodState::IDLE:
        Idle();
        break;
    case ThumpodState::DEAD:
        break;
    }

    return Enemy::Update(dt);
}


bool Thumpod::CleanUp() {
    return Enemy::CleanUp();
}

void Thumpod::Idle() {
    currentAnimation = &idleAnim;
}

void Thumpod::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
        currentState = ThumpodState::DEAD;
        break;
    }
}