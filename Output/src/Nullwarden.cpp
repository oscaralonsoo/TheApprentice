#include "NullwardenSpear.h"
#include "Nullwarden.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"


Nullwarden::Nullwarden() : Enemy(EntityType::NULLWARDEN) {
}

Nullwarden::~Nullwarden() {
}

bool Nullwarden::Awake() {
    return Enemy::Awake();
}

bool Nullwarden::Start() {
    //Add a physics to an item - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texW / 2, (int)position.getY() + texH / 2, texW, texH, bodyType::DYNAMIC);

    //Assign collider type
    pbody->ctype = ColliderType::ENEMY;

    pbody->listener = this;

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node enemyNode = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
    {
        if (std::string(enemyNode.attribute("type").as_string()) == type)
        {
            texture = Engine::GetInstance().textures.get()->Load(enemyNode.attribute("texture").as_string());
            idleAnim.LoadAnimations(enemyNode.child("idle"));
            attackAnim.LoadAnimations(enemyNode.child("attack"));
            chargeAnim.LoadAnimations(enemyNode.child("charge"));
            impaledAnim.LoadAnimations(enemyNode.child("impaled"));
            roarAnim.LoadAnimations(enemyNode.child("roar"));
            deathAnim.LoadAnimations(enemyNode.child("death"));
        }
    }

    b2Fixture* fixture = pbody->body->GetFixtureList();
    if (fixture) {
        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_ATTACK | CATEGORY_PLAYER_DAMAGE;
        fixture->SetFilterData(filter);
    }

    currentAnimation = &idleAnim;

    return true;
}

bool Nullwarden::Update(float dt) {
    switch (currentState)
    {
    case NullwardenState::IDLE:
        if (currentAnimation != &idleAnim) currentAnimation = &idleAnim;
        currentState = NullwardenState::ATTACK;
        break;
    case NullwardenState::ATTACK:
        if (currentAnimation != &attackAnim) {
            currentAnimation = &attackAnim;
            spearIntervalTimer.Start();
            spearAttackTimer.Start();
            direction = -direction;
        }

        if (spearIntervalTimer.ReadMSec() >= spearIntervalMs) {
            SpawnHorizontalSpears();
            spearIntervalTimer.Start();
        }

        if (spearAttackTimer.ReadMSec() >= spearAttackMs) {
           currentState = NullwardenState::CHARGE;
        }
        break;
    case NullwardenState::CHARGE:
        if (currentAnimation != &chargeAnim) currentAnimation = &chargeAnim;
        
        pbody->body->SetLinearVelocity(b2Vec2(direction * 12.0f, 0.0f));
        
        break;
    case NullwardenState::IMPALED:
        if (currentAnimation != &impaledAnim) {
            currentAnimation = &impaledAnim;
            impaledTimer.Start();
        }

        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);

        if (impaledTimer.ReadMSec() >= impaledMs) {
            currentState = NullwardenState::ROAR;
        }
        else if (verticalSpearTimer.ReadMSec() >= verticalSpearIntervalMs && spawnedVerticalSpears < maxVerticalSpears) {
            SpawnVerticalSpears();
            verticalSpearTimer.Start();
        }
        break;
    case NullwardenState::ROAR:
        if (currentAnimation != &roarAnim) {
            currentAnimation = &roarAnim;
            roarTimer.Start();
        }

        if (roarTimer.ReadMSec() >= roarMs) {
            currentState = NullwardenState::ATTACK;
        }
        break;
    case NullwardenState::DEATH:
        if (currentAnimation != &deathAnim) currentAnimation = &deathAnim;
        break;
    }

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(),
        1.0f,
        0.0,
        INT_MAX,
        INT_MAX,
        (direction < 0) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
    currentAnimation->Update();

    return true;
}

bool Nullwarden::PostUpdate() {
    Enemy::PostUpdate();
    if (currentState == NullwardenState::DEATH && currentAnimation->HasFinished()) {
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }

    return true;
}

bool Nullwarden::CleanUp() {
    return Enemy::CleanUp();
}

void Nullwarden::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
        if (currentState == NullwardenState::ATTACK) {
            currentState = NullwardenState::ROAR;
        } else {
            lives--;
            currentState = (lives > 0) ? NullwardenState::ROAR : NullwardenState::DEATH;
        }
        break;
    case ColliderType::PLAYER:
        if (currentState == NullwardenState::ATTACK) currentState = NullwardenState::ROAR;
        break;
    case ColliderType::WALL:
        if (currentState == NullwardenState::CHARGE) currentState = NullwardenState::IMPALED;
        break;
    }

}

void Nullwarden::SpawnHorizontalSpears() {
    const float baseX = position.getX() + 200.0f * direction;
    const float baseY = position.getY() + 20;
    const int totalPositions = 4;
    const float gap = 100.0f;

    std::vector<float> spearPositions;

    for (int i = 0; i < totalPositions; ++i) {
        spearPositions.push_back(baseY - (gap * i));
    }

    int gapIndex = rand() % totalPositions;

    for (int i = 0; i < totalPositions; ++i) {
        if (i == gapIndex) continue;

        Engine::GetInstance().entityManager->AddEntity(new NullwardenSpear(baseX, spearPositions[i], 10.0f, b2Vec2(direction, 0.0f)));
    }
}

void Nullwarden::SpawnVerticalSpears() {
    float baseY = position.getY() + texH;
    float spearX = position.getX() + ((direction < 0) ? -50.0f : texW + 50.0f) + (spawnedVerticalSpears * verticalSpearGap * (direction < 0 ? -1 : 1));

    Engine::GetInstance().entityManager->AddEntity(
        new NullwardenSpear(spearX, baseY, 10.0f, b2Vec2(0.0f, -1.0f))
    );

    spawnedVerticalSpears++;
}

