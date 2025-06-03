#include "Stalactite.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Player.h"

Stalactite::Stalactite() : Entity(EntityType::STALACTITE) {
    name = "Stalactite";
}

Stalactite::~Stalactite() {}

bool Stalactite::Awake() {
    return true;
}

bool Stalactite::Start() {
    pugi::xml_document loadFile;
    loadFile.load_file("config.xml");

    pugi::xml_node node = loadFile.child("config").child("scene").child("animations").child("props").child("stalactite");

    std::string textureAttrName = "texture" + std::to_string(variant);
    texture = Engine::GetInstance().textures->Load(node.attribute(textureAttrName.c_str()).as_string());

    idleAnim.LoadAnimations(node.child("idle"));
    fallAnim.LoadAnimations(node.child("fall"));
    splashAnim.LoadAnimations(node.child("splash"));

    texW = node.attribute("w").as_int();
    texH = node.attribute("h").as_int();

    pbody = Engine::GetInstance().physics->CreateRectangle( position.getX() + texW / 2, position.getY() + texH / 2, texW, texH,bodyType::DYNAMIC );

    if (pbody) {
        pbody->ctype = ColliderType::ENEMY;
        pbody->listener = this;
        pbody->body->SetGravityScale(0);

        if (b2Fixture* fixture = pbody->body->GetFixtureList()) {
            b2Filter filter;
            filter.categoryBits = CATEGORY_ENEMY;
            filter.maskBits = CATEGORY_PLATFORM | CATEGORY_PLAYER | CATEGORY_PLAYER_DAMAGE;

            fixture->SetDensity(1.0f);
            fixture->SetFriction(0.3f);
            fixture->SetRestitution(0.0f);
            fixture->SetFilterData(filter);
        }
    }

    triggerZone = { (int)position.getX() - 20, (int)position.getY() + texH, texW + 40, 800 };
    pbody->body->GetFixtureList()->SetSensor(true);
    currentAnimation = &idleAnim;

    return true;
}

bool Stalactite::Update(float dt) {
    if (!pbody) return false;

    const b2Vec2 velocity = pbody->body->GetLinearVelocity();
    const Vector2D playerPos = Engine::GetInstance().scene->GetPlayer()->GetPosition();
    playerPoint = { (int)playerPos.getX(), (int)playerPos.getY() };

    switch (state) {
    case StalactiteState::IDLE:
        currentAnimation = &idleAnim;
        if (SDL_PointInRect(&playerPoint, &triggerZone)) {
            state = StalactiteState::FALLING;
            pbody->body->SetGravityScale(5.0f);
        }
        break;

    case StalactiteState::FALLING:
        pbody->body->GetFixtureList()->SetSensor(false);
        currentAnimation = &fallAnim;
        if (velocity.y == 0.0f) {
            pbody->body->ApplyForceToCenter(b2Vec2(0, 0.1f), true);
        }
        break;

    case StalactiteState::SPLASHED:
        if (currentAnimation != &splashAnim) {
            currentAnimation = &splashAnim;
            currentAnimation->Reset();
        }

        pbody->body->SetGravityScale(0);
        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);
        break;
    }

    const b2Transform& pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    if (currentAnimation) {
        Engine::GetInstance().render->DrawTexture( texture, (int)position.getX(),(int)position.getY(),&currentAnimation->GetCurrentFrame());
        currentAnimation->Update();
    }

    return true;
}

bool Stalactite::PostUpdate() { 
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) {
        Engine::GetInstance().render->DrawRectangle(triggerZone, 255, 0, 0, 100);
    }
    if (state == StalactiteState::SPLASHED && currentAnimation->HasFinished()) {
        toDelete = true;
    }
    return true;
}

bool Stalactite::CleanUp() {
    if (pbody) {
        Engine::GetInstance().physics->DeletePhysBody(pbody);
        pbody = nullptr;
    }
    return true;
}

void Stalactite::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
            state = StalactiteState::SPLASHED;
        break;
    case ColliderType::PLATFORM:
    case ColliderType::ATTACK:
    case ColliderType::WALL:
    case ColliderType::DOOR:
    case ColliderType::ENEMY:
    case ColliderType::BOX:
        if (state == StalactiteState::FALLING) {
            state = StalactiteState::SPLASHED;
        }
        break;
    }
}
