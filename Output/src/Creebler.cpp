#include "Creebler.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"


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

    return Enemy::Start();
}

bool Creebler::Update(float dt) {
    switch (currentState)
    {
    case CreeblerState::WALKING:
        if (currentAnimation != &walkAnim) currentAnimation = &walkAnim;
        Walk(dt);
        break;
    case CreeblerState::DEAD:
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

void Creebler::Walk(float dt) {
    pbody->body->SetLinearVelocity(b2Vec2(direction * speed * dt, pbody->body->GetLinearVelocity().y));

    Vector2D mapPos = Engine::GetInstance().map.get()->WorldToMap(position.getX() + texW / 2, position.getY() + texH / 2);

    int frontX = mapPos.x + direction;
    int frontY = mapPos.y + 1;

    MapLayer* navigationLayer = Engine::GetInstance().map.get()->GetNavigationLayer();

    if (navigationLayer->Get(frontX, mapPos.y) || !navigationLayer->Get(frontX, frontY)) {
        direction *= -1;
    }
}


void Creebler::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
        currentState = CreeblerState::DEAD;
        break;
    }
}