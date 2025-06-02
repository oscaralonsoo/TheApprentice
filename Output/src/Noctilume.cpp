#include "Noctilume.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Entity.h"
#include "Log.h"
#include "Audio.h"
#include <cmath>

Noctilume::Noctilume() : Enemy(EntityType::NOCTILUME) {}

Noctilume::~Noctilume() {}

bool Noctilume::Awake() {
    return true;
}

bool Noctilume::Start() {
    pugi::xml_document configDoc;
    if (!configDoc.load_file("config.xml")) return false;

    const std::string typeName = "Noctilume";
    for (auto node : configDoc.child("config").child("scene").child("animations").child("enemies").children("enemy")) {
        if (node.attribute("type").as_string() == typeName) {
            texture = Engine::GetInstance().textures->Load(node.attribute("texture").as_string());
            flyingAnim.LoadAnimations(node.child("idle"));
            attackAnim.LoadAnimations(node.child("attack"));
            crashAnim.LoadAnimations(node.child("crash"));
            dieAnim.LoadAnimations(node.child("die"));
            currentAnimation = &flyingAnim;
            break;
        }
    }

    int verticalOffset = 50; 
    pbody = Engine::GetInstance().physics->CreateCircle(
        static_cast<int>(position.x + texH / 2),
        static_cast<int>(position.y + texH / 2 - verticalOffset),
        texH / 4,
        bodyType::DYNAMIC
    );

    originalPosition = position;
    smoothedPosition = position;
    maxSteps = 12;

    soundAttackId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Noctilume/noctilume_attack.ogg", 1.0f);
    soundDeadId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Noctilume/noctilume_death.ogg", 1.0f);
    soundCrashId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Noctilume/noctilume_crash.ogg", 1.0f);
    soundWalkId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Noctilume/noctilume_walk.ogg", 1.0f);

    return Enemy::Start();
}

bool Noctilume::Update(float dt) {
    CheckState();

    playerPos = Engine::GetInstance().scene->GetPlayerPosition();

    if (playerPos.x < position.x) direction = -1;
    else direction = 1;

    switch (currentState) {
    case NoctilumeState::IDLE: Idle(dt); break;
    case NoctilumeState::CHASING: 
        walkSoundTimer -= dt;
        if (walkSoundTimer <= 0.0f) {
            Engine::GetInstance().audio->PlayFx(soundWalkId, 1.0f, 0);
            walkSoundTimer = walkSoundInterval;
        }
        deadSoundPlayed = false;
        crashSoundPlayed = false;
        attackSoundPlayed = false;
        Chasing(dt); break;
    case NoctilumeState::PRE_ATTACK: PreAttack(dt); break;
    case NoctilumeState::ATTACK:
        if (!attackSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundAttackId, 1.0f, 0);
            attackSoundPlayed = true;
        }
        deadSoundPlayed = false;
        crashSoundPlayed = false;
        walkSoundPlayed = false;
        Attack(dt); break;
    case NoctilumeState::CRASH:
        if (!crashSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundCrashId, 1.0f, 0);
            crashSoundPlayed = true;
        }
        deadSoundPlayed = false;
        attackSoundPlayed = false;
        walkSoundPlayed = false;
        Crash(dt); break;
    case NoctilumeState::DEAD: 
        if (!deadSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundDeadId, 1.0f, 0);
            deadSoundPlayed = true;
        }
        attackSoundPlayed = false;
        crashSoundPlayed = false;
        walkSoundPlayed = false;
        
        Die(); break;
    }

    if (currentState != NoctilumeState::CRASH && currentState != NoctilumeState::DEAD) {
        smoothedPosition = smoothedPosition + (position - smoothedPosition) * smoothingSpeed * dt;
    }
    pbody->body->SetTransform(b2Vec2(smoothedPosition.x / PIXELS_PER_METER, smoothedPosition.y / PIXELS_PER_METER),0);

    return Enemy::Update(dt);
}

bool Noctilume::PostUpdate() {
    if (currentState == NoctilumeState::DEAD && currentAnimation && currentAnimation->HasFinished())
    {
        pbody->body->GetFixtureList()->SetSensor(true);
        Engine::GetInstance().entityManager->DestroyEntity(this);
    }
    return true;
}

bool Noctilume::CleanUp() {
    return Enemy::CleanUp();
}

void Noctilume::OnCollision(PhysBody* physA, PhysBody* physB) {
    Enemy::OnCollision(physA, physB);

    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        if (currentState == NoctilumeState::ATTACK && isDiving) {
            currentState = NoctilumeState::CRASH;
            crashTimer = 0.0f;
            isDiving = false;
        }
        break;
    case ColliderType::PLAYER_DAMAGE:
        if (currentState == NoctilumeState::ATTACK && isDiving)
            isDiving = false;
        break;
    case ColliderType::ATTACK:
        if (currentState == NoctilumeState::CRASH)
            currentState = NoctilumeState::DEAD;
        break;
    }
}

void Noctilume::Idle(float dt) {
    currentAnimation = &flyingAnim;
    timePassed += dt;

    float pingPong = fmod(timePassed * waveFrequency * 2 * horizontalRange, 2 * horizontalRange);
    float x = (pingPong < horizontalRange) ? pingPong : (2 * horizontalRange - pingPong);

    position.x = originalPosition.x + x;
    position.y = originalPosition.y + sin(timePassed * waveFrequency + waveOffset) * waveAmplitude;
}

void Noctilume::Chasing(float dt) { 
    currentAnimation = &flyingAnim;
    timePassed += dt;

    Vector2D currentPos = GetBodyPosition();

    delayedPlayerX += (playerPos.x - delayedPlayerX);
    delayedPlayerY += (playerPos.y - delayedPlayerY);

    currentPos.y += (delayedPlayerY - hoverHeight - currentPos.y) * 0.5f;

    float sinValue = sin(oscillationSpeed * timePassed + 2.0f);
    currentPos.x = delayedPlayerX + oscillationAmplitude * sinValue;

    if (lastSinValue < 0 && sinValue >= 0 && !passedZero) {
        oscillationCount++;
        passedZero = true;

        if (oscillationCount >= oscillationsBeforeAttack) {
            currentState = NoctilumeState::PRE_ATTACK;
            anticipationTimer = 0.0f;
            oscillationCount = 0;
            return;
        }
    }

    if (sinValue < 0) passedZero = false;

    originalPosition = position;
    lastSinValue = sinValue;
    position = currentPos;
}

void Noctilume::PreAttack(float dt) {
    currentAnimation = &flyingAnim;
    anticipationTimer += dt;

    if (anticipationTimer == dt) {
        preAttackStartPos = GetBodyPosition();
        preAttackEndPos = preAttackStartPos + Vector2D(0.0f, -4.0f);

        float jumpSpeed = (preAttackEndPos.y - preAttackStartPos.y) / (anticipationDuration / 2.0f);
        pbody->body->SetLinearVelocity(b2Vec2(0, jumpSpeed / PIXELS_PER_METER));
    }

    if (anticipationTimer >= anticipationDuration) {
        pbody->body->SetLinearVelocity(b2Vec2_zero);

        currentState = NoctilumeState::ATTACK;
        attackTarget = playerPos;
        diveStartPos = GetBodyPosition();
        diveDirection = (attackTarget - diveStartPos).Normalized();
        isDiving = true;
        diveProgress = 0.0f;
    }
}

void Noctilume::Attack(float dt) {
    currentAnimation = &attackAnim;

    if (isDiving) {
        position = position + diveDirection * attackSpeed * dt * 100.0f;
    }
    else {
        Vector2D currentPos = GetBodyPosition();
        currentPos.y -= returnSpeed * dt * 100.0f;

        if (currentPos.y <= diveStartPos.y) {
            currentPos.y = diveStartPos.y;
            currentState = NoctilumeState::CHASING;
        }
        position = currentPos;
    }

}

void Noctilume::Crash(float dt) {
    currentAnimation = &crashAnim;

    pbody->body->SetLinearVelocity(b2Vec2_zero);
    pbody->body->SetAngularVelocity(0);

    crashTimer += dt;

    if (crashTimer >= 3000.0f) {
        currentState = pathfinding->HasFoundPlayer() ? NoctilumeState::CHASING : NoctilumeState::IDLE;
        crashTimer = 0.0f;
    }
}

void Noctilume::Die() {
    currentAnimation = &dieAnim;

    if (pbody && pbody->body) {
        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);

        if (pbody->body->GetFixtureList())
            pbody->body->GetFixtureList()->SetSensor(true);
    }
}

void Noctilume::CheckState() {
    if (currentState == NoctilumeState::ATTACK ||
        currentState == NoctilumeState::CRASH ||
        currentState == NoctilumeState::DEAD ||
        currentState == NoctilumeState::PRE_ATTACK) return;

    if (pathfinding->HasFoundPlayer()) {
        currentState = NoctilumeState::CHASING;
    }
    else {
        if (currentState != NoctilumeState::IDLE) {
            currentState = NoctilumeState::IDLE;
            pbody->body->SetLinearVelocity(b2Vec2_zero);
            pbody->body->SetAngularVelocity(0);
        }
    }
}

Vector2D Noctilume::GetBodyPosition() const {
    if (pbody && pbody->body)
        return Vector2D(METERS_TO_PIXELS(pbody->body->GetPosition().x), METERS_TO_PIXELS(pbody->body->GetPosition().y));
    return position;
}
