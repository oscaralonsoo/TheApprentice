
#include "Broodheart.h"
#include "Brood.h"
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

    spawnInterval = 4500.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2500.0f;

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
    for (Brood* brood : broodsAlive) {
        brood->SetParent(nullptr);
    }
    broodsAlive.clear();
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
    if (broodsAlive.size() < 6) {
        std::vector<SDL_FPoint> newPositions;

        for (int i = 0; i < 2; ++i) {
            if (broodsAlive.size() >= 6) break;

            SDL_FPoint spawnPos;
            bool validPos = false;

            int attempts = 0;
            while (!validPos && attempts < 10) {
                float angle = (float)(rand() % 360) * M_PI / 180.0f;
                float distance = 60.0f + (rand() / (float)RAND_MAX) * (spawnRadius - 60.0f);

                float offsetX = cosf(angle) * distance;
                float offsetY = sinf(angle) * distance;

                spawnPos.x = position.x + offsetX;
                spawnPos.y = position.y + offsetY;

                validPos = true;
                for (const SDL_FPoint& pos : newPositions) {
                    float dx = spawnPos.x - pos.x;
                    float dy = spawnPos.y - pos.y;
                    float distSq = dx * dx + dy * dy;
                    if (distSq < 2500.0f) { 
                        validPos = false;
                        break;
                    }
                }
                attempts++;
            }

            if (!validPos) continue; 

            newPositions.push_back(spawnPos);

            pugi::xml_document tempDoc;
            pugi::xml_node enemyNode = tempDoc.append_child("enemy");

            enemyNode.append_attribute("type") = type.c_str();
            enemyNode.append_attribute("x") = spawnPos.x;
            enemyNode.append_attribute("y") = spawnPos.y;
            enemyNode.append_attribute("w") = texW / 2;
            enemyNode.append_attribute("h") = texH / 2;
            enemyNode.append_attribute("gravity") = false;

            Brood* child = (Brood*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BROOD);
            child->SetParameters(enemyNode);
            child->Start();
            child->SetParent(this);

            broodsAlive.push_back(child);
        }
    }
}

void Broodheart::OnBroodDeath(Brood* brood)
{
    broodsAlive.remove(brood);
    spawnCooldown = 0.0f;
}


