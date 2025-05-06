#include "Thumpod.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"

Thumpod::Thumpod() : Enemy(EntityType::THUMPOD) {
    direction = 1.0f;
    previousDirection = direction;
}
Thumpod::~Thumpod() {}

bool Thumpod::Awake() {
    return Enemy::Awake();
}

bool Thumpod::Start() {
    pugi::xml_document loadFile;
    if (!loadFile.load_file("config.xml")) return false;

    auto enemies = loadFile.child("config").child("scene").child("animations").child("enemies");
    for (auto enemyNode : enemies.children("enemy")) {
        if (enemyNode.attribute("type").as_string() == type) {
            texture = Engine::GetInstance().textures->Load(enemyNode.attribute("texture").as_string());
            idleAnim.LoadAnimations(enemyNode.child("idle"));
            attackUpAnim.LoadAnimations(enemyNode.child("attackUp"));
            attackDownAnim.LoadAnimations(enemyNode.child("attackDown"));
            deadAnim.LoadAnimations(enemyNode.child("dead"));
            break;
        }
    }

   shape = {
        {PIXEL_TO_METERS(40), PIXEL_TO_METERS(-45)},
        {PIXEL_TO_METERS(-105), PIXEL_TO_METERS(75)},
        {PIXEL_TO_METERS(85), PIXEL_TO_METERS(75)}

    };

    pbody = Engine::GetInstance().physics->CreatePolygon((int)position.getX(), (int)position.getY(), shape, bodyType::DYNAMIC);
    pbody->ctype = ColliderType::ENEMY;
    pbody->listener = this;
    if (!gravity) pbody->body->SetGravityScale(1);

    pathfinding = new Pathfinding();
    ResetPath();

    b2Fixture* fixture = pbody->body->GetFixtureList();
    if (fixture) {
        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_ATTACK | CATEGORY_PLAYER_DAMAGE;
        fixture->SetFilterData(filter);
    }
    return true;
}

bool Thumpod::Update(float dt) {
    jumpCooldown += dt;
    UpdateDirectionFacingPlayer();
    switch (currentState) {
    case ThumpodState::IDLE:
        Idle();
        break;

    case ThumpodState::ATTACK:
        Attack(dt);
        break;

    case ThumpodState::DEAD:
        Die();
        break;
    }

    return Enemy::Update(dt);
}

bool Thumpod::PostUpdate() {
    Enemy::PostUpdate();
    if (currentState == ThumpodState::DEAD && currentAnimation->HasFinished())
        Engine::GetInstance().entityManager->DestroyEntity(this);
    return true;
}

bool Thumpod::CleanUp() {
    return Enemy::CleanUp();
}

void Thumpod::Idle() {
    currentAnimation = &idleAnim;
    if (pathfinding->HasFoundPlayer() && jumpCooldown >= jumpInterval)
        currentState = ThumpodState::ATTACK;
}

void Thumpod::Attack(float dt) {
    float mass = pbody->body->GetMass();

    Vector2D target = Engine::GetInstance().map->MapToWorld(
        pathfinding->pathTiles.front().getX(),
        pathfinding->pathTiles.front().getY()
    );

    TryStartJump(mass);
    TryChangeToFallingState(mass);
    TryFinishAttackDown(dt);
}

void Thumpod::TryStartJump(float mass) {
    if (!hasJumped && jumpCooldown >= jumpInterval) {
        currentAnimation = &attackUpAnim;
        hasJumped = isJumpingUp = true;
        isOnGround = isFallingTowardsPlayer = hasLanded = false;
        canPlayAttackDownAnim = true;
        jumpCooldown = 0.0f;

        pbody->body->ApplyLinearImpulseToCenter({ 0, jumpForceY * mass }, true);
    }
}

void Thumpod::TryChangeToFallingState(float mass) {
    if (isJumpingUp && pbody->body->GetLinearVelocity().y > 0) {
        isJumpingUp = false;
        isFallingTowardsPlayer = true;
        pbody->body->ApplyLinearImpulseToCenter({ 8.0f * direction * mass, 0 }, true);
    }
}

void Thumpod::TryFinishAttackDown(float dt) {
    if (hasLanded && currentAnimation == &attackDownAnim) {
        attackDownTimer += dt;
        if (attackDownTimer >= minAttackDownTime && currentAnimation->HasFinished()) {
            pbody->body->SetLinearVelocity({ 0, 0 });
            ResetAttackState();
            currentState = ThumpodState::IDLE;
        }
    }
}

void Thumpod::ResetAttackState() {
    hasJumped = isOnGround = isJumpingUp = isFallingTowardsPlayer = hasLanded = isPlayingAttackDown = false;
    attackDownTimer = jumpCooldown = 0.0f;
}

void Thumpod::Die() {
    currentAnimation = &deadAnim;
    pbody->body->SetLinearVelocity(b2Vec2_zero);
    pbody->body->SetAngularVelocity(0);
    pbody->body->GetFixtureList()->SetSensor(true);
}

void Thumpod::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        isOnGround = true;
        [[fallthrough]];

    case ColliderType::PLAYER:
        if (canPlayAttackDownAnim) {
            currentAnimation = &attackDownAnim;
            canPlayAttackDownAnim = false;
            hasLanded = isPlayingAttackDown = true;
            attackDownTimer = 0.0f;
        }
        break;

    case ColliderType::ATTACK:
        currentState = ThumpodState::DEAD;
        break;
    }
}
void Thumpod::UpdateDirectionFacingPlayer()
{
    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();
    float newDirection = (playerPos.getX() > position.getX()) ? 1.0f :
        (playerPos.getX() < position.getX()) ? -1.0f : direction;

    if (newDirection != direction)
    {
        previousDirection = direction;
        direction = newDirection;

        // Flip el cuerpo
        Engine::GetInstance().physics->FlipPhysBody(pbody, true, false);
    }
}

