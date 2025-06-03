#include "Creebler.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Audio.h"


Creebler::Creebler() : Enemy(EntityType::CREEBLER) {
}

Creebler::~Creebler() {
}

bool Creebler::Awake() {
    return Enemy::Awake();
}

bool Creebler::Start() {
    //Add a physics to an item - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texW / 2, (int)position.getY() + texH / 2, texW / 1.3, texH / 2.2, bodyType::DYNAMIC, 0, -6);

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node enemyNode = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
    {
        if (std::string(enemyNode.attribute("type").as_string()) == type)
        {
            texture = Engine::GetInstance().textures.get()->Load(enemyNode.attribute("texture").as_string());
            walkAnim.LoadAnimations(enemyNode.child("walk"));
            deathAnim.LoadAnimations(enemyNode.child("death"));
        }
    }

    currentAnimation = &walkAnim;

    soundWalkId = Engine::GetInstance().audio->LoadFx("Assets/Audio/fx/Creebler/creebler_walk.ogg", 1.0f);
    soundDeadId = Engine::GetInstance().audio->LoadFx("Assets/Audio/fx/Creebler/creebler_death.ogg", 1.0f);
    maxSteps = 0;
    return Enemy::Start();
}

bool Creebler::Update(float dt) {
    switch (currentState)
    {
    case CreeblerState::WALKING:
        if (currentAnimation != &walkAnim) currentAnimation = &walkAnim;
        Walk();

        walkSoundTimer -= dt;
        if (walkSoundTimer <= 0.0f) {
            Engine::GetInstance().audio->PlayFx(soundWalkId, 1.0f, 0);
            walkSoundTimer = walkSoundInterval;  
        }

        deadSoundPlayed = false;
        break;
    case CreeblerState::DEAD:
        if (!deadSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundDeadId, 1.0f, 0);
            deadSoundPlayed = true;
        }
        walkSoundPlayed = false;
        if (currentAnimation != &deathAnim) currentAnimation = &deathAnim;

        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);
        pbody->body->GetFixtureList()->SetSensor(true);
        pbody->body->SetGravityScale(0);
        break;
    }

    return Enemy::Update(dt);
}


bool Creebler::PostUpdate() {
    if (currentState == CreeblerState::DEAD && currentAnimation->HasFinished()) {
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }

    return true;
}

bool Creebler::CleanUp() {
    return Enemy::CleanUp();
}

void Creebler::Walk() {
    pbody->body->SetLinearVelocity(b2Vec2(direction * speed, pbody->body->GetLinearVelocity().y));

    Vector2D posMap = Engine::GetInstance().map.get()->WorldToMap(position.getX() + texW / 2, position.getY() + texH / 2);

    int frontX = posMap.x + direction;
    int frontY = posMap.y + 1;

    MapLayer* layer = nullptr;

    if (!navigationLayerName.empty()) {
        layer = Engine::GetInstance().map->GetNavigationLayerByName(navigationLayerName);
    }
    else {
        layer = Engine::GetInstance().map->GetNavigationLayer();
    }

    if (!layer) return; // fallback de seguridad


    if (navigationId)
    {
        if ((layer->Get(frontX, posMap.y) == navigationId) || !layer->Get(frontX, frontY))
            direction *= -1;
    }
    else {
        if ((layer->Get(frontX, posMap.y)) || !layer->Get(frontX, frontY))
            direction *= -1;
    }


}

void Creebler::OnCollision(PhysBody* physA, PhysBody* physB)
{
    Enemy::OnCollision(physA, physB);

    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
        currentState = CreeblerState::DEAD;
        break;
    }
}