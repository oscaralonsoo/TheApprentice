#include "Dreadspire.h"
#include "DreadspireBullet.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Entity.h"
#include "Log.h"
#include "Audio.h"
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

    int offsetY = -3; // Valor por defecto
    int offsetX = -20;
    switch (rotationAngle) {
    case 90:
        offsetY = -15;
        break;
    case 180:
        offsetY = 25;
        break;
    case 270:
        offsetY = 45;
        offsetX = 15;
        break;
    case 0 :
        offsetY = -3;
        offsetX = 20;
        break;
    }

    // Crear el cuerpo con dimensiones correctas
    pbody = Engine::GetInstance().physics.get()->CreateRectangle(
        (int)position.getX() + texW / 2,
        (int)position.getY() + texH / 2,
        64,
        100,
        bodyType::DYNAMIC,
        offsetX,
        offsetY

    );

    // Establecer la rotación física del cuerpo
    float radians = DEGTORAD * rotationAngle;
    b2Vec2 center = pbody->body->GetPosition(); // centro actual
    pbody->body->SetTransform(center, radians);

    maxSteps = 25;

    soundAttackId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Dreadspire/dreadspire_recharge.ogg", 1.0f);
    soundDeadId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Dreadspire/dreadspire_death.ogg", 1.0f);
    soundRechargeId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Dreadspire/dreadspire_attack.ogg", 1.0f);
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
    case DreadspireState::RECHARGING: Recharge(dt);
        if (!rechargeSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundRechargeId, 0.5f, 0);
            rechargeSoundPlayed = true;
        }
        deadSoundPlayed = false;
        attackSoundPlayed = false;
        
        break;
    case DreadspireState::SHOOTING: Shoot(dt);
        if (!attackSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundAttackId, 0.5f, 0);
            attackSoundPlayed = true;
        }
        deadSoundPlayed = false;
        rechargeSoundPlayed = false;
        break;
    case DreadspireState::DEAD: currentAnimation = &dieAnim; 
        if (!deadSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundDeadId, 0.5f, 0);
            deadSoundPlayed = true;
        }
        attackSoundPlayed = false;
        rechargeSoundPlayed = false;
        break;
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
    Enemy::OnCollision(physA, physB);

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

    if (currentAnimation->HasFinished()) {
        b2Vec2 toPlayer = b2Vec2(playerPos.x - position.x, playerPos.y - position.y);
        baseAngle = atan2(toPlayer.y, toPlayer.x);

        float centerX = METERS_TO_PIXELS(pbody->body->GetPosition().x);
        float centerY = METERS_TO_PIXELS(pbody->body->GetPosition().y);

        float spread = M_PI / 4; 
        float angles[3] = { -spread, 0.0f, spread };

        float extendedSpawnOffset = spawnOffset; 

        for (int i = 0; i < 3; ++i) {
            float shootingAngle = baseAngle + angles[i];
            b2Vec2 dir = b2Vec2(cos(shootingAngle), sin(shootingAngle));

            float offsetX = centerX + dir.x * extendedSpawnOffset;
            float offsetY = centerY + dir.y * extendedSpawnOffset;

            auto bullet = new DreadspireBullet(offsetX, offsetY, 12.0f, dir);
            Engine::GetInstance().entityManager->AddEntity(bullet);
        }

        currentAnimation->Reset();
        rechargeTimer = 0.0f;
        currentState = DreadspireState::RECHARGING;
    }
}

void Dreadspire::Recharge(float dt)
{
    currentAnimation = &idleAnim;

    rechargeTimer += dt;
    if (rechargeTimer >= rechargeCooldown) {
        rechargeTimer = 0.0f;
        currentState = DreadspireState::SHOOTING;
    }
}

void Dreadspire::CheckState() {
    if (currentState == DreadspireState::DEAD || currentState == DreadspireState::RECHARGING)
        return;

    if (pathfinding->HasFoundPlayer()) {
        currentState = DreadspireState::SHOOTING;
    }
    else {
        currentState = DreadspireState::IDLE;
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