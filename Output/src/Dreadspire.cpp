#include "Dreadspire.h"
#include "DreadspireBullet.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Entity.h"
#include "Log.h"
#include <cmath>

Dreadspire::Dreadspire() : Enemy(EntityType::DREADSPIRE) {}

Dreadspire::~Dreadspire() {}

bool Dreadspire::Awake() {
    return true;
}

bool Dreadspire::Start() {
    pugi::xml_document configDoc;
    if (!configDoc.load_file("config.xml")) return false;

    const std::string typeName = "Dreadspire";
    for (auto node : configDoc.child("config").child("scene").child("animations").child("enemies").children("enemy")) {
        if (node.attribute("type").as_string() == typeName) {
            texture = Engine::GetInstance().textures->Load(node.attribute("texture").as_string());
            texH = node.attribute("h").as_int();
            texW = node.attribute("w").as_int();
            idleAnim.LoadAnimations(node.child("idle"));
            shootingAnim.LoadAnimations(node.child("shooting"));
            dieAnim.LoadAnimations(node.child("die"));
            currentAnimation = &idleAnim;
            break;
        }
    }

    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texW / 2, (int)position.getY() + texH / 2, 64, 100 , bodyType::DYNAMIC, 20, -3);
  
    maxSteps = 15;

    return Enemy::Start();
}

bool Dreadspire::Update(float dt) {
    pbody->body->SetLinearVelocity(b2Vec2(0, 0));

    CheckState();

    playerPos = Engine::GetInstance().scene->GetPlayerPosition();

    if (playerPos.x < position.x) direction = -1;
    else direction = 1;

    switch (currentState) {
    case DreadspireState::IDLE: Idle(dt); break;
    case DreadspireState::RECHARGING: Recharge(dt); break;
    case DreadspireState::SHOOTING: Shoot(dt); break;
    case DreadspireState::DEAD: currentAnimation = &dieAnim; break;
    }

    ChangeBodyType();

    return Enemy::Update(dt);
}


bool Dreadspire::PostUpdate() {
    if (currentState == DreadspireState::DEAD && currentAnimation->HasFinished())
        Engine::GetInstance().entityManager->DestroyEntity(this);
    return true;
}

bool Dreadspire::CleanUp() {
    return Enemy::CleanUp();
}
void Dreadspire::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::ATTACK:
        currentState = DreadspireState::DEAD;
        break;
    case ColliderType::PLAYER:

        shouldBecomeStatic = true;  
        break;
    }
}
void Dreadspire::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLAYER:
        shouldBecomeDynamic = true; 
        break;
    }
}
void Dreadspire::Idle(float dt) {
    currentAnimation = &idleAnim;
}

void Dreadspire::Shoot(float dt)
{
    currentAnimation = &shootingAnim;

    static float fireCooldown = 0;
    fireCooldown -= dt;

    if (fireCooldown > 0) return;

    if (bulletsShot == 0 && bulletShootTimer <= 0.0f) {
        bulletShootTimer = 0.001f; 
    }

    if (bulletShootTimer > 0.0f) {
        bulletShootTimer += dt;
        if (bulletsShot == 0) {
            b2Vec2 toPlayer = b2Vec2(playerPos.x - position.x, playerPos.y - position.y);
            baseAngle = atan2(toPlayer.y, toPlayer.x);
        }
        if (bulletsShot < 3 && bulletShootTimer >= (200 * (bulletsShot + 1))) {
         
            float shootingAngle = baseAngle + angles[bulletsShot];
            b2Vec2 dir = b2Vec2(cos(shootingAngle), sin(shootingAngle));


            float centerX = METERS_TO_PIXELS(pbody->body->GetPosition().x);
            float centerY = METERS_TO_PIXELS(pbody->body->GetPosition().y);

            float offsetX = centerX + dir.x * spawnOffset;
            float offsetY = centerY + dir.y * spawnOffset;

            auto bullet = new DreadspireBullet(offsetX, offsetY, 12.0f, dir);
            Engine::GetInstance().entityManager->AddEntity(bullet);

            bulletsShot++;
        }

        if (bulletsShot >= 3) {
            bulletShootTimer = 0.0f;
            bulletsShot = 0;
            fireCooldown = 2500.0f;

            if (currentAnimation->HasFinished()) {
                currentState = DreadspireState::RECHARGING;
            }
        }
    }
}


void Dreadspire::Recharge(float dt)
{
    currentAnimation = &idleAnim;
}

void Dreadspire::CheckState() {
    if(currentState == DreadspireState::DEAD)
        return;

    if (pathfinding->HasFoundPlayer()) {
        currentState = DreadspireState::SHOOTING;
    }
    else {
        if (currentState != DreadspireState::IDLE) {
            currentState = DreadspireState::IDLE;
        }
    }
}

void Dreadspire::ChangeBodyType() {
    if (shouldBecomeStatic && pbody && pbody->body) {
        pbody->body->SetType(b2_staticBody);
        shouldBecomeStatic = false;
    }
    else if (shouldBecomeDynamic && pbody && pbody->body) {
        pbody->body->SetType(b2_dynamicBody);
        shouldBecomeDynamic = false;
    }
}