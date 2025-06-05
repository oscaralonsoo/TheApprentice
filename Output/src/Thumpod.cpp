#include "Thumpod.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Audio.h"

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
    maxSteps = 15;

    soundJumpId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Thumpod/thumpod_jump.ogg", 1.0f);
    soundDeadId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Thumpod/thumpod_death.ogg", 1.0f);

    return Enemy::Start();
}

bool Thumpod::Update(float dt) {
    jumpCooldown += dt;
    UpdateDirectionFacingPlayer();

    switch (currentState) {
    case ThumpodState::IDLE:
        Idle();
        break;

    case ThumpodState::ATTACK:

        if (!jumpSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundJumpId, 0.5f, 0);
            jumpSoundPlayed = true;
        }

        Attack(dt);
        break;

    case ThumpodState::DEAD:
        if (!deadSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundDeadId, 0.5f, 0);
            deadSoundPlayed = true;
        }
        Die();
        break;
    }

    ResetPath();

    steps = 0;
    while (pathfinding->pathTiles.empty() && steps < maxSteps) {
        pathfinding->PropagateAStar(SQUARED);
        steps++;
    }

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY() - 8, &currentAnimation->GetCurrentFrame(),
        1.0f,
        0.0,
        INT_MAX,
        INT_MAX,
        (direction < 0) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL,
        scale
    );
    currentAnimation->Update();

    //Show|Hide Path
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) {
        showPath = !showPath;
    }
    if (showPath) {
        pathfinding->DrawPath();
    }

    return true;
}

bool Thumpod::PostUpdate() {
    if (currentState == ThumpodState::DEAD && currentAnimation->HasFinished())
        Engine::GetInstance().entityManager->DestroyEntity(this);
    return true;
}
bool Thumpod::CleanUp() {
    return Enemy::CleanUp();
}

void Thumpod::OnCollision(PhysBody* physA, PhysBody* physB) {
    Enemy::OnCollision(physA, physB);

    switch (physB->ctype) {
    case ColliderType::PLATFORM:
    case ColliderType::WALL:
    {
        b2Vec2 velocity = pbody->body->GetLinearVelocity();

        if (velocity.y >= 0.0f) {
            isOnGround = true;

            if (canPlayAttackDownAnim && isFallingTowardsPlayer) {
                currentAnimation = &attackDownAnim;
                canPlayAttackDownAnim = false;
                hasLanded = isPlayingAttackDown = true;
                attackDownTimer = 0.0f;
            }
        }
    }
    break;

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

void Thumpod::Idle() {
    currentAnimation = &idleAnim;
    if (pathfinding->HasFoundPlayer() && jumpCooldown >= jumpInterval && currentState != ThumpodState::ATTACK)
    {
        currentState = ThumpodState::ATTACK;
    }
    else if (currentState != ThumpodState::IDLE)
    {
        currentState = ThumpodState::IDLE;
    }
}

void Thumpod::Attack(float dt) {
    float mass = pbody->body->GetMass();
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
        if (attackDownTimer >= minAttackDownTime && currentAnimation && currentAnimation->HasFinished()) {
            pbody->body->SetLinearVelocity({ 0, 0 });
            ResetAttackState();
                jumpSoundPlayed = false;
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
    if (pbody && pbody->body) {
        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);
        if (pbody->body->GetFixtureList())
            pbody->body->GetFixtureList()->SetSensor(true);
    }
}

void Thumpod::UpdateDirectionFacingPlayer() {
    float playerX = Engine::GetInstance().scene->GetPlayerPosition().getX();
    float thumpodX = position.getX();

    float desiredDirection = (playerX > thumpodX) ? 1.0f : (playerX < thumpodX) ? -1.0f : direction;

    if (desiredDirection != direction) {
        previousDirection = direction;
        direction = desiredDirection;
        Engine::GetInstance().physics->FlipPhysBody(pbody, true, false);
    }
}
