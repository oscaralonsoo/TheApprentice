#include "NullwardenSpear.h"
#include "Nullwarden.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Player.h"
#include "PressureSystemController.h"

Nullwarden::Nullwarden() : Enemy(EntityType::NULLWARDEN) {
}

Nullwarden::~Nullwarden() {
}

bool Nullwarden::Awake() {
    return Enemy::Awake();
}

bool Nullwarden::Start() {
    //Add a physics to an item - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX(),(int)position.getY(), texW, texH , bodyType::DYNAMIC, CATEGORY_ENEMY, CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_ATTACK | CATEGORY_PLAYER_DAMAGE);

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
            impaledAnim1.LoadAnimations(enemyNode.child("impaled1"));
            crystalAppearAnim1.LoadAnimations(enemyNode.child("crystalAppear1"));
            impaledAnim2.LoadAnimations(enemyNode.child("impaled2"));
            crystalAppearAnim2.LoadAnimations(enemyNode.child("crystalAppear2"));
            impaledAnim3.LoadAnimations(enemyNode.child("impaled3"));
            crystalAppearAnim3.LoadAnimations(enemyNode.child("crystalAppear3"));
            roarAnim.LoadAnimations(enemyNode.child("roar"));
            hitAnim.LoadAnimations(enemyNode.child("hit"));
            deathAnim.LoadAnimations(enemyNode.child("death"));
        }
    }

    crystal = new NullwardenCrystal(position.getX(), position.getY(), 0.0f, b2Vec2_zero, this);
    Engine::GetInstance().entityManager->AddEntity(crystal);
    drawY = (int)position.getY();
    drawX = (int)position.getX();
    currentAnimation = &idleAnim;

    direction = -1;
    return true;
}

bool Nullwarden::Update(float dt) {
    if (currentState != previousState) {
        if (currentState == NullwardenState::ATTACK) {
            spearAttackTimer.Start(); 
            attackAnimDone = false;   
        }
        previousState = currentState; 
    }
    switch (currentState)
    {
    case NullwardenState::IDLE:
        if (currentAnimation != &idleAnim) currentAnimation = &idleAnim;
        currentState = NullwardenState::ATTACK;
        break;
    case NullwardenState::ATTACK:
        Attack();
        break;
    case NullwardenState::CHARGE:
        if (currentAnimation != &chargeAnim)
        {
            beforeChargeTimer.Start();
            currentAnimation = &chargeAnim;
        }
        if (beforeChargeTimer.ReadMSec() >= beforeChargeMs)
        {
            startedImpaledAnim = false;
            pbody->body->SetLinearVelocity(b2Vec2(direction * 12.0f, 0.0f));
        }

        break;
    case NullwardenState::IMPALED:
        if (!startedImpaledAnim) Roar();
        
        Impaled();
        break;
    case NullwardenState::ROAR:
        if (!changedDirection)
        {
            direction = -direction;
            changedDirection = true;
        }
        if (currentAnimation != &roarAnim) {
            roarAnim.Reset();
            currentAnimation = &roarAnim;
            startedImpaledAnim = false;
        }
        Roar();
        if (currentAnimation->HasFinished()) {
            currentState = NullwardenState::ATTACK;
        }
        break;

    case NullwardenState::DEATH:
        if (currentAnimation != &hitAnim) { currentAnimation = &hitAnim; }

        if (currentAnimation != &deathAnim && currentAnimation->HasFinished()) currentAnimation = &deathAnim;
        break;
    }
    if (currentAnimation != previousAnimation) {
        UpdateColliderSizeToCurrentAnimation();
        previousAnimation = currentAnimation;
    }

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    UpdateDraw();
    Engine::GetInstance().render.get()->DrawTexture(texture, drawX , drawY, &currentAnimation->GetCurrentFrame(),
        1.0f,
        0.0,
        INT_MAX,
        INT_MAX,
        (direction < 0) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);

    currentAnimation->Update();
    return true;
}

bool Nullwarden::PostUpdate() {
    if (currentState == NullwardenState::DEATH && currentAnimation == &deathAnim && currentAnimation->HasFinished()) {

        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        Engine::GetInstance().pressureSystem->OpenDoor(2);
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
        if (!crystalBroken && currentState == NullwardenState::IMPALED) {

            Engine::GetInstance().render->StartCameraShake(0.2f, 3);
            currentState = NullwardenState::ROAR;
            break;
        }
        if (currentState == NullwardenState::ATTACK) {
            Roar();
        }
        else if(crystalBroken && currentState == NullwardenState::IMPALED){
            currentState = NullwardenState::DEATH;
            pbody->body->GetFixtureList()->SetSensor(true);
        }
        break;
    case ColliderType::PLAYER:
        if (currentState == NullwardenState::ATTACK) Roar();
        break;
    case ColliderType::WALL:
        if (currentState == NullwardenState::CHARGE) currentState = NullwardenState::IMPALED;
        break;
    }
}

void Nullwarden::SpawnHorizontalSpears() {
    float baseX;
    if (direction > 0)
    {
        baseX = position.getX() + 350.0f * direction;
    }
    else {
        baseX = position.getX() + 150.0f * direction;
    }
    const float baseY = position.getY() + 230.0f;
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
    float baseY = position.getY() + 400.0f;
    float spearX = position.getX() + ((direction > 0) ? -50.0f : texW + 50.0f) + (spawnedVerticalSpears * verticalSpearGap * (direction > 0 ? -1 : 1));

    Engine::GetInstance().entityManager->AddEntity(
        new NullwardenSpear(spearX, baseY, 10.0f, b2Vec2(0.0f, -1.0f))
    );

    if (spawnedVerticalSpears >= maxVerticalSpears)
    {
        spawnedVerticalSpears = 0;
    }
    else {
        spawnedVerticalSpears++;
    }
}

void Nullwarden::Attack() {
    if (!attackAnimDone) {
        attackAnim.Reset();
        currentAnimation = &attackAnim;
        attackAnimDone = true;
    }
    if (!currentAnimation->HasFinished()) {
        return;
    }
    if (currentAnimation->HasFinished()) {
        SpawnHorizontalSpears();
        attackAnimDone = false;
        if(spearAttackTimer.ReadMSec() == 0)
        {
            spearAttackTimer.Start();
        }
    }

    if (spearAttackTimer.ReadMSec() >= spearAttackMs) {

        currentState = NullwardenState::CHARGE;
        changedDirection = false;
        
    }
}
void Nullwarden::Impaled() {
    pbody->body->SetLinearVelocity(b2Vec2_zero);
    pbody->body->SetAngularVelocity(0);

    if (!startedImpaledAnim)
        ChangeImpaledAnim();

    else {
        if (impaledTimer.ReadMSec() >= impaledMs) {
            currentState = NullwardenState::ROAR;
            spawnedVerticalSpears = 0;
        }
        else if (verticalSpearTimer.ReadMSec() >= verticalSpearIntervalMs && spawnedVerticalSpears <= maxVerticalSpears) {
            SpawnVerticalSpears();
            verticalSpearTimer.Start();
        }
    }
}
void Nullwarden::Roar() {

    
    Player* player = Engine::GetInstance().scene->GetPlayer();

    float playerX = player->position.getX();
    float playerY = player->position.getY();
    float nullwardenX = this->position.getX();
    float nullwardenY = this->position.getY();

    float dx = playerX - nullwardenX;
    float dy = playerY - nullwardenY;

    float distanceSquared = dx * dx + dy * dy;
    const float roarRadius = 3500.0f;
    const float roarRadiusSquared = roarRadius * roarRadius;

    if (distanceSquared <= roarRadiusSquared) {
        float distance = sqrtf(distanceSquared);
        float falloff = 1.0f - (distance / roarRadius);

        if (falloff < 0.0f) falloff = 0.0f;
        else if (falloff > 1.0f) falloff = 1.0f;

        float pushDir = (dx > 0.0f) ? 1.0f : -1.0f;
        float pushStrength = 35.0f * falloff;

        b2Vec2 push = b2Vec2(pushDir * pushStrength, 0);
        player->pbody->body->SetAngularVelocity(0);
        player->pbody->body->ApplyLinearImpulseToCenter(push, true);

        Engine::GetInstance().render->StartCameraShake(0.2f * falloff, 6 * falloff);
    }
   
}
void Nullwarden::ChangeImpaledAnim() {
    if (crystalBroken)
    {
        if (currentAnimation != &impaledAnim) {
            currentAnimation = &impaledAnim;
            impaledTimer.Start();
            startedImpaledAnim = true;
        }
        return;
    }
    switch (crystal->currentState) {
    case CrystalState::PRISTINE:
        if (currentAnimation != &crystalAppearAnim1) {
            crystalAppearAnim1.Reset();
            currentAnimation = &crystalAppearAnim1;
        }
        else if (currentAnimation->HasFinished()) {
            currentAnimation = &impaledAnim1;
            impaledTimer.Start();
            startedImpaledAnim = true;
        }
        break;

    case CrystalState::CRACKED:
        if (currentAnimation != &crystalAppearAnim2) {
            crystalAppearAnim2.Reset();
            currentAnimation = &crystalAppearAnim2;
        }
        else if (currentAnimation->HasFinished()) {
            currentAnimation = &impaledAnim2;
            impaledTimer.Start();
            startedImpaledAnim = true;
        }
        break;

    case CrystalState::SHATTERED:
        if (currentAnimation != &crystalAppearAnim3) {
            crystalAppearAnim3.Reset();
            currentAnimation = &crystalAppearAnim3;
        }
        else if (currentAnimation->HasFinished()) {
            currentAnimation = &impaledAnim3;
            impaledTimer.Start();
            startedImpaledAnim = true;
        }
        break;
    }
}
void Nullwarden::UpdateDraw() {
    drawY = (int)position.getY();
    drawX = (int)position.getX();

    if (currentAnimation == &attackAnim) {
        drawY -= 150;
        drawX += (direction < 0) ? -200 : -100;
    }
    else if (currentState == NullwardenState::IMPALED) 
    {
        drawX += (direction < 0) ? -80 : -50;
    }
    else if (currentAnimation == &chargeAnim) {
        drawY += 50;
        drawX += (direction < 0) ? -60 : -60;
    }
    else if (currentAnimation == &roarAnim) {
        drawX += (direction < 0) ? -50 : -50;
    }
    else if (currentAnimation == &deathAnim)
    {
        drawY -= 50;
        drawX += (direction < 0) ? -60 : -60;
    }
}

void Nullwarden::UpdateColliderSizeToCurrentAnimation() {
    if (!pbody || !pbody->body) return;

    float width = 0.0f;
    float height = 0.0f;

    if (currentAnimation == &attackAnim || currentAnimation == &idleAnim || currentAnimation == &roarAnim ||
        currentAnimation == &deathAnim || currentAnimation == &hitAnim) {
        width = PIXEL_TO_METERS(texW);
        height = PIXEL_TO_METERS(texH);
    }

    else {
        SDL_Rect frame = currentAnimation->GetCurrentFrame();
        width = PIXEL_TO_METERS(frame.w);
        height = PIXEL_TO_METERS(frame.h);
    }

    b2Fixture* fixture = pbody->body->GetFixtureList();
    if (!fixture) return;
        
    pbody->body->DestroyFixture(fixture);

    b2PolygonShape newShape;
    newShape.SetAsBox(width / 2.0f, height / 2.0f);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &newShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.filter.categoryBits = CATEGORY_ENEMY;
    fixtureDef.filter.maskBits = CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_ATTACK | CATEGORY_PLAYER_DAMAGE;

    pbody->body->CreateFixture(&fixtureDef);
}



