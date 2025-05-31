#include "DreadspireBullet.h"
#include "Engine.h"
#include "EntityManager.h"
#include "Scene.h"

DreadspireBullet::DreadspireBullet(float x, float y, float speed, b2Vec2 direction)
    : Entity(EntityType::BULLET), direction(direction), speed(speed)
{
    position = Vector2D(x, y);
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node node = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); node; node = node.next_sibling("enemy"))
    {
        if (std::string(node.attribute("type").as_string()) == "DreadspireBullet")
        {
            texture = Engine::GetInstance().textures.get()->Load(node.attribute("texture").as_string());
            width = node.attribute("w").as_int();
            height = node.attribute("h").as_int();
            idleAnim.LoadAnimations(node.child("idle"));
        }
    }

    pbody = Engine::GetInstance().physics->CreateCircle(position.x, position.y, width/2, bodyType::DYNAMIC);
    pbody->ctype = ColliderType::ENEMY;
    pbody->listener = this;

    pbody->body->SetGravityScale(0.0f);

    if (b2Fixture* fixture = pbody->body->GetFixtureList()) {
        fixture->SetSensor(false);

        b2Filter filter;
        filter.categoryBits = CATEGORY_PROJECTILE;
        filter.maskBits = CATEGORY_WALL | CATEGORY_PLAYER_DAMAGE | CATEGORY_PLATFORM;
        fixture->SetFilterData(filter);
    }

    pbody->body->SetLinearVelocity(b2Vec2(direction.x * speed, direction.y * speed));
 
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
    case ColliderType::WALL_SLIDE:
    case ColliderType::DESTRUCTIBLE_WALL:
    case ColliderType::SPIKE:
    case ColliderType::BOX:
    case ColliderType::PLAYER:
        Engine::GetInstance().entityManager->DestroyEntity(this);
        break;
    default:
        break;
    }
}
