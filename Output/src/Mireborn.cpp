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
            walkAnim.LoadAnimations(enemyNode.child("walk"));
        }
    }

    currentAnimation = &idleAnim;

    return Enemy::Start();
}

bool Mireborn::Update(float dt) {
    if (pathfinding->HasFoundPlayer()) {
        if (currentState == MirebornState::IDLE) {
            currentState = MirebornState::WALKING;
        }
    }
    GetPosition();
    switch (currentState)
    {
    case MirebornState::IDLE:
        Idle(dt);
        break;
    case MirebornState::WALKING:
        Walk(dt);
        break;
    }
    return Enemy::Update(dt);
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
        // Damage the player
        break;
    case ColliderType::ATTACK:
        Divide();
        break;
    }

}

void Mireborn::Idle(float dt) {
    currentAnimation = &idleAnim;

    jumpCooldown += dt;

    if (jumpCooldown >= 1.0f) {
        currentState = MirebornState::WALKING;
        jumpCooldown = 0.0f;
    }
}

void Mireborn::Walk(float dt)
{
    currentAnimation = &walkAnim;
    jumpCooldown += dt;

    // Verify Jump
    if (!hasJumped && jumpCooldown >= jumpInterval)
    {
        isOnGround = false;
        hasJumped = true;
        jumpCooldown = 0.0f;

        Vector2D nextTile = pathfinding->pathTiles.front();
        Vector2D nextTileWorld = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

        float direction = (nextTileWorld.getX() > position.getX()) ? 1.0f :
            (nextTileWorld.getX() < position.getX() ? -1.0f : 0.0f);

        pbody->body->ApplyLinearImpulseToCenter(b2Vec2(jumpForceX * direction, jumpForceY), true);
    }
    // Verify Grounded
    if (isOnGround) {
        pbody->body->SetLinearVelocity(b2Vec2(0, 0));
        hasJumped = false;
        jumpCooldown = 0.0f;
        isOnGround = false;
    }
}
void Mireborn::Divide() {
    Enemy* clone = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::MIREBORN);
    Vector2D offset(20, 0);
    clone->SetPosition(this->GetPosition() + offset);
    clone->Start();


}
