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

    return Enemy::Start();
}

bool Broodheart::Update(float dt) {
    spawnCooldown += dt;

    if (spawnCooldown >= spawnInterval) {
        shouldSpawn = true;
        spawnCooldown = 0.0f;
    }

    return Enemy::Update(dt);
}
bool Broodheart::PostUpdate() {
    if (shouldSpawn) {
        Spawn();
        shouldSpawn = false;
    }

    return Enemy::PostUpdate();
}
bool Broodheart::CleanUp() {
    return Enemy::CleanUp();
}

void Broodheart::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
        break;
    case ColliderType::PLAYER:

        break;
    case ColliderType::ATTACK:
        
        break;
    }
}

void Broodheart::Spawn() {
    const float spawnRadius = 100.0f;


    // Solo permite el spawn si hay menos de 6 Broods
    if (broodCount < 6) {
        for (int i = 0; i < 2; ++i) {
            if (broodCount >= 6) break; // Salir si ya hay 6 Broods

            float angle = (float)(rand() % 360) * M_PI / 180.0f;
            float distance = 40.0f + (rand() / (float)RAND_MAX) * (spawnRadius - 40.0f);

            float offsetX = cosf(angle) * distance;
            float offsetY = sinf(angle) * distance;

            pugi::xml_document tempDoc;
            pugi::xml_node enemyNode = tempDoc.append_child("enemy");

            enemyNode.append_attribute("type") = type.c_str();
            enemyNode.append_attribute("x") = position.x + offsetX;
            enemyNode.append_attribute("y") = position.y + offsetY;
            enemyNode.append_attribute("w") = texW / 2;
            enemyNode.append_attribute("h") = texH / 2;
            enemyNode.append_attribute("gravity") = true;

            Enemy* child = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BROOD);
            child->SetParameters(enemyNode);
            child->Start();

            // Incrementar el contador de Broods después de crear uno
            broodCount++;
        }
    }
}


