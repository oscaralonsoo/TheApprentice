#include "Shyver.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Audio.h"


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
            preattackAnim.LoadAnimations(enemyNode.child("preAttack"));
            stunAnim.LoadAnimations(enemyNode.child("stun"));
            waitAnim.LoadAnimations(enemyNode.child("wait"));
            deathAnim.LoadAnimations(enemyNode.child("death"));
            invisibleAnim.LoadAnimations(enemyNode.child("invisible"));
            waveAnim.LoadAnimations(enemyNode.child("wave"));
        }
    }

    currentAnimation = &attackAnim;
    maxSteps = 5;
    soundWalkId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Shyver/shyver_walk.ogg", 1.0f);
    soundDashId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Shyver/shyver_dash.ogg", 1.0f);
    soundDeadId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Shyver/shyver_death.ogg", 1.0f);
    soundAppearId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Shyver/shyver_appear.ogg", 1.0f);


    return Enemy::Start();
}

bool Shyver::Update(float dt) {
    switch (currentState)
    {
    case ShyverState::IDLE:
        if (currentAnimation != &idleAnim) currentAnimation = &idleAnim;
        if(pathfinding->HasFoundPlayer())
        currentState = ShyverState::APPEAR;
        break;
    case ShyverState::APPEAR:
        if (currentAnimation != &appearAnim) {
            currentAnimation = &appearAnim;
            currentAnimation->Reset();
        }
        if (!appearSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundAppearId, 0.5f, 0);
            appearSoundPlayed = true;
        }
        walkSoundPlayed = false;
        dashSoundPlayed = false;
        deadSoundPlayed = false;
        appearSoundPlayed = false;
        Wait();
        if (currentAnimation->HasFinished()) currentState = ShyverState::WAIT;
        break;
    case ShyverState::WAIT:
        if (currentAnimation != &idleAnim) {
            currentAnimation = &idleAnim;
            waitTimer.Start();
        }
        Wait();
        if (waitTimer.ReadMSec() >= waitDuration && currentAnimation->currentFrame >= currentAnimation->totalFrames - 1) {
            currentState = ShyverState::PRE_ATTACK; 
        }
        break;

    case ShyverState::PRE_ATTACK:
        if (currentAnimation != &preattackAnim) {
            currentAnimation = &preattackAnim;
            currentAnimation->Reset();
        }

        if (currentAnimation->HasFinished()) {
            attackInProgress = false; 
            currentState = ShyverState::ATTACK;
        }
        break;
    case ShyverState::ATTACK:
        if (currentAnimation != &attackAnim) {
            currentAnimation = &attackAnim;
            currentAnimation->Reset();
        }
        if (!dashSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundDashId, 0.5f, 0);
            dashSoundPlayed = true;
        }
        walkSoundPlayed = false;
        deadSoundPlayed = false;
        appearSoundPlayed = false;
        Attack();
        break;
    case ShyverState::STUNNED:
        if (currentAnimation != &stunAnim) {
            currentAnimation = &stunAnim;
            stunTimer.Start();
        }
        Stun();
        if (stunTimer.ReadMSec() >= stunDuration) currentState = ShyverState::DISAPPEAR;
        break;
    case ShyverState::DISAPPEAR:
        if (currentAnimation != &disappearAnim) {
            currentAnimation = &disappearAnim;
            currentAnimation->Reset();
        }
        if (!appearSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundAppearId, 0.5f, 0);
            appearSoundPlayed = true;
        }
        walkSoundPlayed = false;
        dashSoundPlayed = false;
        deadSoundPlayed = false;
        Disappear();
        if (currentAnimation->HasFinished()) currentState = ShyverState::INVISIBLE;
        break;
    case ShyverState::INVISIBLE:
        if (currentAnimation != &invisibleAnim) {
            currentAnimation = &invisibleAnim;
            direction = -direction;
            invisibleTimer.Start();
        }
        Invisible();
        break;
    case ShyverState::DEATH:
        if (!deadSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundDeadId, 0.5f, 0);
            deadSoundPlayed = true;
        }
        walkSoundPlayed = false;
        dashSoundPlayed = false;
        appearSoundPlayed = false;
        if (currentAnimation != &deathAnim) currentAnimation = &deathAnim;

        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);
        break;
    }

    direction = -direction;
    return Enemy::Update(dt);
}


bool Shyver::PostUpdate() {
    if (printWave)
    {
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (direction > 0) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
        if (rotationAngle == 180) flip = (SDL_RendererFlip)(flip | SDL_FLIP_NONE);

        secondAnimation = &waveAnim;
        Engine::GetInstance().render.get()->DrawTexture(texture,
            (int)waveAnimStartPos.getX()-200,
            (int)waveAnimStartPos.getY()-275,
            &secondAnimation->GetCurrentFrame(),
            1.0f,
            (double)rotationAngle,
            INT_MAX,
            INT_MAX,
            flip,
            scale
        );
        secondAnimation->Update();
    }

    if (currentState == ShyverState::DEATH && currentAnimation->HasFinished()) {
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }
    direction = -direction;
    Enemy::PostUpdate();
    return true;
}

bool Shyver::CleanUp() {
    return Enemy::CleanUp();
}

void Shyver::Wait() {
    static float delayedPlayerX = 0.0f;
    static float delayedPlayerY = 0.0f;

    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();

    delayedPlayerX += (playerPos.x - delayedPlayerX) * 0.1f;
    delayedPlayerY += (playerPos.y - delayedPlayerY) * 0.1f;

    Vector2D playerOffset;

    if (direction > 0)
    {
        playerOffset = Vector2D(delayedPlayerX - 400.0f, delayedPlayerY - 200.0f);
    }
    else {
        playerOffset = Vector2D(delayedPlayerX + 200.0f, delayedPlayerY - 200.0f);
    }
    SetPosition(playerOffset);
}

void Shyver::Attack() {
    const float maxDistance = 650.0f;
    const float maxSpeed = 23.0f;
    const float minSpeed = 5.0f;

    if (!attackInProgress) {
        attackStartX = GetPosition().x;
        attackInProgress = true;
    }

    float distanceMoved = std::abs(GetPosition().x - attackStartX);
    float t = distanceMoved / maxDistance;
    float speed = minSpeed + (maxSpeed - minSpeed) * std::sin(t * M_PI);
    if (attackAnim.currentFrame > 3.0f)
    {
        if (!printWave) {
            waveAnimStartPos = GetPosition();
            waveAnim.Reset();
        }
        printWave = true;
        pbody->body->SetLinearVelocity(b2Vec2(direction * speed, 0));
    }
    if (distanceMoved >= maxDistance) {
        currentState = ShyverState::STUNNED;
        pbody->body->SetLinearVelocity(b2Vec2_zero);
        attackInProgress = false;
    }
}

void Shyver::Stun() {
    printWave = false;
    pbody->body->SetLinearVelocity(b2Vec2_zero);

}

void Shyver::Disappear() {
    
}

void Shyver::Death() {

}

void Shyver::Invisible() {
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    
    if (invisibleTimer.ReadMSec() >= invisibleDuration) currentState = ShyverState::APPEAR;
}

void Shyver::OnCollision(PhysBody* physA, PhysBody* physB)
{
    Enemy::OnCollision(physA, physB);

    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
        if (currentState == ShyverState::STUNNED) currentState = ShyverState::DEATH;
        
        break;
    }
}

