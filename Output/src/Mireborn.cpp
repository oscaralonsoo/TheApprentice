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
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 3, texH / 3, bodyType::DYNAMIC);

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node enemyNode = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
    {
        if (std::string(enemyNode.attribute("type").as_string()) == type)
        {
            texture = Engine::GetInstance().textures.get()->Load(enemyNode.attribute("texture").as_string());
            idleAnim.LoadAnimations(enemyNode.child("idle"));
            walkAnim.LoadAnimations(enemyNode.child("walking"));
            divideAnim.LoadAnimations(enemyNode.child("divide"));
            deathAnim.LoadAnimations(enemyNode.child("death"));
        }
    }

    currentAnimation = &idleAnim;

    scale = (tier == "Alpha") ? 1.0f :
            (tier == "Beta")  ? 0.75f :
            (tier == "Gamma") ? 0.5f :
                                1.0f;
    maxSteps = 15;
    return Enemy::Start();
}

bool Mireborn::Update(float dt) {
    pbody->body->SetGravityScale(0.5);

    switch (currentState)
    {
    case MirebornState::IDLE:
        if (currentAnimation != &idleAnim) currentAnimation = &idleAnim;
        Idle(dt);
        break;
    case MirebornState::WALKING:
        Walk(dt);
        break;
    case MirebornState::DIVIDING:
        if (currentAnimation != &divideAnim) currentAnimation = &divideAnim;
        pbody->body->SetLinearVelocity(b2Vec2(0, 0));
        break;
    case MirebornState::DEATH:
        if (currentAnimation != &deathAnim) currentAnimation = &deathAnim;
        pbody->body->SetLinearVelocity(b2Vec2(0, 0));
        break;
    }
    return Enemy::Update(dt);
}

bool Mireborn::PostUpdate() {
    if ((currentState == MirebornState::DIVIDING || currentState == MirebornState::DEATH) && currentAnimation->HasFinished()) {
        if (currentState == MirebornState::DIVIDING) {
            Divide();
        }
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }
    return true;
}

bool Mireborn::CleanUp() {
    return Enemy::CleanUp();
}

void Mireborn::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        isOnGround = true;
        break;
    case ColliderType::PLAYER:
        break;
    case ColliderType::ATTACK:
        if (tier == "Gamma")
        {
            currentState = MirebornState::DEATH;
        }
        else {
            currentState = MirebornState::DIVIDING;
        }
        break;
    }
}

void Mireborn::Idle(float dt) {

    if (jumpCooldownTimer.ReadMSec() > jumpCooldown)
    {
        currentState = MirebornState::WALKING;
    }
}

void Mireborn::Walk(float dt)
{
    if (pathfinding->pathTiles.empty()) {
        if (isOnGround) {
            currentState = MirebornState::IDLE;
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            jumpCooldownTimer.Start();
            currentAnimation->Reset();
        }
        return;
    }

    if (currentAnimation != &walkAnim) {
        currentAnimation = &walkAnim;

        Vector2D nextTile = pathfinding->pathTiles.front();
        Vector2D nextTileWorld = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

        float direction = (nextTileWorld.getX() > position.getX()) ? 1.0f :
            (nextTileWorld.getX() < position.getX() ? -1.0f : 0.0f);

        float mass = pbody->body->GetMass();
        pbody->body->ApplyLinearImpulseToCenter(
            b2Vec2(jumpForceX * direction * mass, jumpForceY * mass), true
        );

        isOnGround = false;
    }

    if (isOnGround) {
        currentState = MirebornState::IDLE;
        pbody->body->SetLinearVelocity(b2Vec2(0, 0));
        jumpCooldownTimer.Start();
        currentAnimation->Reset();
    }
}

void Mireborn::Divide() {
    for (int i = 0; i < MAX_DIVIDES; ++i) {
        pugi::xml_document tempDoc;
        pugi::xml_node enemyNode = tempDoc.append_child("enemy");

        enemyNode.append_attribute("type") = type.c_str();
        enemyNode.append_attribute("x") = position.x + ((i == 0) ? 30 : -30);
        enemyNode.append_attribute("y") = position.y;
        enemyNode.append_attribute("w") = texW / 1.5;
        enemyNode.append_attribute("h") = texH / 1.5;
        enemyNode.append_attribute("gravity") = true;
        if (tier == "Alpha")
        {
            enemyNode.append_attribute("tier") = "Beta";
        }
        else if(tier == "Beta") {
            enemyNode.append_attribute("tier") = "Gamma";
        }

        Enemy* clone = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::MIREBORN);
        clone->SetParameters(enemyNode);
        clone->Start();
    }
}