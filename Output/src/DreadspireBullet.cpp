#include "DreadspireBullet.h"
#include "Engine.h"
#include "EntityManager.h"
#include "Scene.h"

DreadspireBullet::DreadspireBullet(float x, float y, float speed, b2Vec2 direction)
    : Entity(EntityType::BULLET), direction(direction), speed(speed)
{
    width = 32;
    height = 32;

    pbody = Engine::GetInstance().physics->CreateCircle(x, y, width/2, bodyType::DYNAMIC);
    pbody->ctype = ColliderType::ENEMY;
    pbody->listener = this;

    pbody->body->SetGravityScale(0.0f);

    if (b2Fixture* fixture = pbody->body->GetFixtureList()) {
        fixture->SetSensor(false);

        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_WALL | CATEGORY_PLAYER_DAMAGE;
        fixture->SetFilterData(filter);
    }

    pbody->body->SetLinearVelocity(b2Vec2(direction.x * speed, direction.y * speed));

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node node = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); node; node = node.next_sibling("enemy"))
    {
        if (std::string(node.attribute("type").as_string()) == "DreadspireBullet")
        {
            texture = Engine::GetInstance().textures.get()->Load(node.attribute("texture").as_string());

            idleAnim.LoadAnimations(node.child("idle"));
        }
    }
    baseDirection = direction;
    perpDirection = b2Vec2(-direction.y, direction.x);
    startPosition = b2Vec2(x, y);
    currentAnimation = &idleAnim;
}

DreadspireBullet::~DreadspireBullet() {
    if (pbody) {
        Engine::GetInstance().entityManager->DestroyEntity(this);
        pbody = nullptr;
    }
    delete currentAnimation;
    currentAnimation = nullptr;
}

bool DreadspireBullet::Update(float dt)
{
    b2Transform pbodyPos = pbody->body->GetTransform();

    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - width / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - height / 2);

    Engine::GetInstance().render->DrawTexture(
        texture,
        (int)position.getX(),
        (int)position.getY(),
        &currentAnimation->GetCurrentFrame()
    );

    currentAnimation->Update();
    time += dt;

    float amplitude = 2.0f;    
    float frequency = 0.005f;     

    // Ondulation
    b2Vec2 linearMovement = baseDirection;
    b2Vec2 waveOffset = perpDirection;
    waveOffset *= sinf(time * frequency) * amplitude / PIXELS_PER_METER;

    b2Vec2 newPos = pbody->body->GetPosition();
    newPos += waveOffset;

    pbody->body->SetTransform(newPos, pbody->body->GetAngle());
    return true;
}

bool DreadspireBullet::CleanUp()
{
    Engine::GetInstance().physics->DeletePhysBody(pbody);
    return true;
}

void DreadspireBullet::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
    case ColliderType::PLATFORM:
    case ColliderType::WALL:
    case ColliderType::PLAYER:
        Engine::GetInstance().entityManager->DestroyEntity(this);
        break;
    }
}
