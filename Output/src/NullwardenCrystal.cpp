#include "NullwardenCrystal.h"
#include "Engine.h"
#include "Enemy.h"
#include "Textures.h"
#include "Render.h"
#include "Physics.h"
#include "Scene.h"
#include "Nullwarden.h"
#include "EntityManager.h"

NullwardenCrystal::NullwardenCrystal(float x, float y, float speed, b2Vec2 dir, Nullwarden* owner, b2Vec2 offset)
    : Entity(EntityType::CRYSTAL), direction(dir), nullwarden(owner), relativeOffset(offset)
{
    width = 80;
    height = 80;

    pbody = Engine::GetInstance().physics->CreateCircle(x, y, width/2, bodyType::DYNAMIC);
    pbody->ctype = ColliderType::ENEMY;
    pbody->listener = this;

    pbody->body->SetGravityScale(0.0f);

    if (b2Fixture* fixture = pbody->body->GetFixtureList()) {
        fixture->SetSensor(false);

        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_WALL |CATEGORY_ATTACK;
        fixture->SetFilterData(filter);
    }

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node node = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); node; node = node.next_sibling("enemy"))
    {
        if (std::string(node.attribute("type").as_string()) == "NullwardenCrystal")
        {
            texture = Engine::GetInstance().textures.get()->Load(node.attribute("texture").as_string());

            pristineAnim.LoadAnimations(node.child("pristine"));
            crackedAnim.LoadAnimations(node.child("cracked"));
            shatteredAnim.LoadAnimations(node.child("shattered"));
            breakAnim.LoadAnimations(node.child("break"));
        }
    }

    currentAnimation = &pristineAnim;
}

NullwardenCrystal::~NullwardenCrystal() {
    if (pbody) {
        Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
        pbody = nullptr;
    }
}
bool NullwardenCrystal::Update(float dt) {
    UpdateCrystalState();

    if (nullwarden && nullwarden->pbody) {
        b2Vec2 nullPos = nullwarden->pbody->body->GetPosition();

        float nullX = METERS_TO_PIXELS(nullPos.x);
        float nullY = METERS_TO_PIXELS(nullPos.y);

        float offsetX = METERS_TO_PIXELS(nullwarden->pbody->body->GetFixtureList()->GetShape()->m_radius) + width / 2;

        //TODO AJUSTAR CRISTAL
        if (nullwarden->direction < 0 && nullwarden->currentState == NullwardenState::IMPALED) {
            pbody->body->GetFixtureList()->SetSensor(false);
            relativeOffset = b2Vec2(3, -1.6f);
            pbody->body->SetTransform(b2Vec2(nullPos.x + relativeOffset.x, nullPos.y + relativeOffset.y), 0.0f);
            pbody->body->SetLinearVelocity(b2Vec2_zero);
            pbody->body->SetAngularVelocity(0);
            direction = b2Vec2(1, 0);
        }
        else if (nullwarden->direction > 0 && nullwarden->currentState == NullwardenState::IMPALED) {
            pbody->body->GetFixtureList()->SetSensor(false);
            relativeOffset = b2Vec2(-3, -1.6f);
            pbody->body->SetTransform(b2Vec2(nullPos.x + relativeOffset.x, nullPos.y + relativeOffset.y), 0.0f);
            pbody->body->SetLinearVelocity(b2Vec2_zero);
            pbody->body->SetAngularVelocity(0);
            direction = b2Vec2(-1, -0);
        }
        else {
            pbody->body->GetFixtureList()->SetSensor(true);
            relativeOffset = b2Vec2(0, 4);
            pbody->body->SetTransform(b2Vec2(nullPos.x + relativeOffset.x, nullPos.y + relativeOffset.y), 0.0f);
            direction = b2Vec2(0, 0);
        }
    }

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - width / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - height / 2);

    currentAnimation->Update();

    SDL_RendererFlip flip = (direction.x < 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    float angle = 0.0f;

    // Animación de ruptura
    if (currentState == CrystalState::BROKEN) {
        if (nullwarden) {
            nullwarden->currentState = NullwardenState::ROAR;
        }
    }

    return true;
}


bool NullwardenCrystal::PostUpdate() {
    if (currentState == CrystalState::BROKEN && currentAnimation->HasFinished())
        Engine::GetInstance().entityManager->DestroyEntity(this);

    return true;
}
bool NullwardenCrystal::CleanUp() {
    if (pbody) {
        Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
        pbody = nullptr;
    }
    return true;
}

void NullwardenCrystal::OnCollision(PhysBody* physA, PhysBody* physB) {
    if (physB->ctype == ColliderType::ATTACK && hits < 4) {
        hits++;
        if(hits < 3){
            nullwarden->currentState = NullwardenState::ROAR;
        }
        Engine::GetInstance().render->StartCameraShake(0.3f, 2);
    }
}
void NullwardenCrystal::UpdateCrystalState() {
    switch (hits) {
    case 1:
        currentState = CrystalState::CRACKED;
        currentAnimation = &crackedAnim;
        break;
    case 2:
        currentState = CrystalState::SHATTERED;
        currentAnimation = &shatteredAnim;
        break;
    case 3:
        currentState = CrystalState::BROKEN;
        currentAnimation = &breakAnim;
        nullwarden->crystalBroken = true;
        nullwarden->startedImpaledAnim = false;
        break;
    }
}
