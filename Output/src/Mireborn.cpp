#include "Mireborn.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"


Mireborn::Mireborn() : Enemy(EntityType::MIREBORN) {
}

Mireborn::~Mireborn() {
}

bool Mireborn::Awake() {
    return Enemy::Awake();
}

bool Mireborn::Start() {
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
        }
    }

    currentAnimation = &idleAnim;

    return Enemy::Start();
}

bool Mireborn::Update(float dt) {
    return Enemy::Update(dt);
}


bool Mireborn::CleanUp() {
    return Enemy::CleanUp();
}


void Mireborn::OnCollision(PhysBody* physA, PhysBody* physB) {

}