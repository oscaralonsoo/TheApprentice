#include "Broodheart.h"
#include "Brood.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Entity.h"
#include "Log.h"
#include "Audio.h"
#include <algorithm>

constexpr int MAX_BROODS = 4;
constexpr int BROODS_PER_SPAWN = 1;
constexpr int MAX_SPAWN_ATTEMPTS = 10;
constexpr float MIN_SPAWN_DISTANCE = 60.0f;
constexpr float MIN_DISTANCE_BETWEEN_BROODS_SQ = 2500.0f;

Broodheart::Broodheart() : Enemy(EntityType::BROODHEART) {}

Broodheart::~Broodheart() {}

bool Broodheart::Awake() {
    return Enemy::Awake();
}

bool Broodheart::Start() {

    spawnInterval = 6500.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 4000.0f;

    pugi::xml_document configDoc;
    if (!configDoc.load_file("config.xml")) {
        LOG("Failed to load config.xml for Broodheart");
        return false;
    }

    std::string typeName = "Broodheart";
    for (pugi::xml_node node : configDoc.child("config").child("scene").child("animations").child("enemies").children("enemy")) {
        if (node.attribute("type").as_string() == typeName) {
            texture = Engine::GetInstance().textures->Load(node.attribute("texture").as_string());
            idleAnim.LoadAnimations(node.child("idle"));
            spawnAnim.LoadAnimations(node.child("spawn"));
            deathAnim.LoadAnimations(node.child("death"));
     
            break;
        }
    }
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW / 2, texH/2, bodyType::DYNAMIC);

    currentAnimation = &idleAnim;
    pbody->ctype = ColliderType::ENEMY;
    if (!gravity) pbody->body->SetGravityScale(0);
    pbody->listener = this;

    pathfinding = new Pathfinding();
    ResetPath();

    b2Fixture* fixture = pbody->body->GetFixtureList();
    if (fixture) {
        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_ATTACK | CATEGORY_PLAYER_DAMAGE;
        fixture->SetFilterData(filter);
    }

    soundSpawnId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Broodheart/broodheart_spawn.ogg", 1.0f);
    soundDeathId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Broodheart/broodheart_death.ogg", 1.0f);

    return true;
}

bool Broodheart::Update(float dt) {
    spawnCooldown += dt;
    if (spawnCooldown >= spawnInterval) {
        shouldSpawn = true;
        spawnCooldown = 0.0f;
    }

    if (shouldSpawn == false && currentAnimation == &spawnAnim) {
        if (!spawnSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundSpawnId, 0.5f, 0);
            spawnSoundPlayed = true;
        }
        if (spawnAnim.HasFinished()) {
            currentAnimation = &idleAnim;
            spawnSoundPlayed = false;
        }
    }
    return Enemy::Update(dt);
}

bool Broodheart::PostUpdate() {
    if (isBroken && currentAnimation == &deathAnim && currentAnimation->HasFinished()) {
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        return true;
    }
    if (shouldSpawn) {
        Spawn();
        shouldSpawn = false;
    }
    return true;
}

bool Broodheart::CleanUp() {
    for (Brood* brood : broodsAlive) {
        brood->SetParent(nullptr);
    }
    broodsAlive.clear();
    return Enemy::CleanUp();
}

void Broodheart::OnCollision(PhysBody* physA, PhysBody* physB) {
    Enemy::OnCollision(physA, physB);

    switch (physB->ctype) {
    case ColliderType::PLAYER:
        break;
    case ColliderType::ATTACK:
        if (!isBroken) {
            isBroken = true;
            currentAnimation = &deathAnim;
            pbody->body->GetFixtureList()->SetSensor(true);
            if (!deathSoundPlayed) {
                Engine::GetInstance().audio->PlayFx(soundDeathId, 0.5f, 0);
                deathSoundPlayed = true;
            }
        }
        break;
    }
}
void Broodheart::Spawn() {
    if (broodsAlive.size() >= MAX_BROODS) return;
    currentAnimation = &spawnAnim;
    std::vector<SDL_FPoint> newPositions;

    for (int i = 0; i < BROODS_PER_SPAWN && broodsAlive.size() < MAX_BROODS; ++i) {
        SDL_FPoint spawnPos;
        bool validPos = false;

        for (int attempt = 0; attempt < MAX_SPAWN_ATTEMPTS && !validPos; ++attempt) {
            float angle = static_cast<float>(rand() % 360) * M_PI / 180.0f;
            float distance = MIN_SPAWN_DISTANCE + static_cast<float>(rand()) / RAND_MAX * (spawnRadius - MIN_SPAWN_DISTANCE);

            spawnPos.x = position.x + cosf(angle) * distance;
            spawnPos.y = position.y + sinf(angle) * distance;

            validPos = std::all_of(newPositions.begin(), newPositions.end(), [&](const SDL_FPoint& pos) {
                float dx = spawnPos.x - pos.x;
                float dy = spawnPos.y - pos.y;
                return (dx * dx + dy * dy) >= MIN_DISTANCE_BETWEEN_BROODS_SQ;
                });
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

        auto* brood = static_cast<Brood*>(Engine::GetInstance().entityManager->CreateEntity(EntityType::BROOD));
        brood->SetParameters(enemyNode);
        brood->Start();
        brood->SetParent(this);


        broodsAlive.push_back(brood);
    }
}

void Broodheart::OnBroodDeath(Brood* brood) {
    broodsAlive.remove(brood);

}
