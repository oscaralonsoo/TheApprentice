#include "Shyver.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"


Shyver::Shyver() : Enemy(EntityType::SHYVER) {
}

Shyver::~Shyver() {
}

bool Shyver::Awake() {
    return Enemy::Awake();
}

bool Shyver::Start() {
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 3, texH / 3, bodyType::DYNAMIC);
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node enemyNode = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
    {
        if (std::string(enemyNode.attribute("type").as_string()) == type)
        {
            texture = Engine::GetInstance().textures.get()->Load(enemyNode.attribute("texture").as_string());
            idleAnim.LoadAnimations(enemyNode.child("idle"));
            disappearAnim.LoadAnimations(enemyNode.child("disappear"));
            appearAnim.LoadAnimations(enemyNode.child("appear"));
            attackAnim.LoadAnimations(enemyNode.child("attack"));
            stunAnim.LoadAnimations(enemyNode.child("stun"));
            deathAnim.LoadAnimations(enemyNode.child("death"));
            deathAnim.LoadAnimations(enemyNode.child("death"));
        }
    }

    currentAnimation = &attackAnim;
    maxSteps = 15;

    return Enemy::Start();
}

bool Shyver::Update(float dt) {
    switch (currentState)
    {
    case ShyverState::IDLE:
        if (currentAnimation != &idleAnim) currentAnimation = &idleAnim;
        currentState = ShyverState::APPEAR;
        break;
    case ShyverState::APPEAR:
        if (currentAnimation != &appearAnim) {
            currentAnimation = &appearAnim;
            appearTimer.Start();
        }

        Appear();

        if (appearTimer.ReadMSec() >= appearDuration) {
            currentState = ShyverState::ATTACK;
        }
        break;
    case ShyverState::ATTACK:
        if (currentAnimation != &attackAnim) currentAnimation = &attackAnim;
        Attack();
        break;
    case ShyverState::STUNNED:
        if (currentAnimation != &stunAnim) currentAnimation = &stunAnim;
        Stun();
        break;
    case ShyverState::DISAPPEAR:
        if (currentAnimation != &disappearAnim) currentAnimation = &disappearAnim;
        Disappear();
        break;
    case ShyverState::INVISIBLE:
        if (currentAnimation != &invisibleAnim) currentAnimation = &invisibleAnim;
        Invisible();
        break;
    case ShyverState::DEATH:
        if (currentAnimation != &deathAnim) currentAnimation = &deathAnim;

        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);
        break;
    }

    return Enemy::Update(dt);
}


bool Shyver::PostUpdate() {
    Enemy::PostUpdate();
    if (currentState == ShyverState::DEATH && currentAnimation->HasFinished()) {
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }

    return true;
}

bool Shyver::CleanUp() {
    return Enemy::CleanUp();
}

void Shyver::Appear() {
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();

    SetPosition(Vector2D(playerPos.x + 90, playerPos.y - 50));
}

void Shyver::Attack() {
    pbody->body->SetLinearVelocity(b2Vec2(direction * 5, 0));
}

void Shyver::Stun() {

}

void Shyver::Disappear() {

}

void Shyver::Death() {

}

void Shyver::Invisible() {

}

void Shyver::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
        currentState = ShyverState::DEATH;
        break;
    }
}

