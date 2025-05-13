#include "DungBeetleBall.h"
#include "Engine.h"
#include "EntityManager.h"
#include "Scene.h"
DungBeetleBall::DungBeetleBall(float x, float y, float speed, b2Vec2 direction)
    : Entity(EntityType::BALL), direction(direction), speed(speed)
{
    width = 128;

    pbody = Engine::GetInstance().physics->CreateCircle(x, y, width / 2, bodyType::DYNAMIC);
    pbody->ctype = ColliderType::ENEMY;
    pbody->listener = this;

    pbody->body->SetGravityScale(0.0f);
    if (b2Fixture* fixture = pbody->body->GetFixtureList()) {
        fixture->SetSensor(false);

        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_WALL | CATEGORY_PLAYER_DAMAGE | CATEGORY_PLATFORM;
        fixture->SetFilterData(filter);

        //Bounce Fixture Settings
        fixture->SetRestitution(1.0f);
        fixture->SetFriction(0.0f);
        pbody->body->SetLinearDamping(0.0f);
        pbody->body->SetAngularDamping(0.0f);
    }

    pbody->body->SetLinearVelocity(b2Vec2(direction.x * speed, direction.y * speed));

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node node = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); node; node = node.next_sibling("enemy"))
    {
        if (std::string(node.attribute("type").as_string()) == "DungBeetleBall")
        {
            texture = Engine::GetInstance().textures.get()->Load(node.attribute("texture").as_string());

            idleAnim.LoadAnimations(node.child("idle"));
        }
    }

    currentAnimation = &idleAnim;
}

DungBeetleBall::~DungBeetleBall()
{
    if (pbody) {
        Engine::GetInstance().entityManager->DestroyEntity(this);
        pbody = nullptr;
    }
    delete currentAnimation;
    currentAnimation = nullptr;
}

bool DungBeetleBall::Update(float dt)
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

bool DungBeetleBall::CleanUp()
{
    Engine::GetInstance().physics->DeletePhysBody(pbody);
    return true;
}

void DungBeetleBall::OnCollision(PhysBody* physA, PhysBody* physB, const b2Vec2& normal)
{
    switch (physB->ctype)
    {
    case ColliderType ::ATTACK:
    case ColliderType::PLATFORM:
    case ColliderType::WALL:
        Bounce(normal);
        break;
    case ColliderType::PLAYER:
        // TODO TONI --- ATRAVESAR EL PLAYER O BOUNCE
        break;
    }
}

void DungBeetleBall::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
    case ColliderType::PLATFORM:
    case ColliderType::WALL:
        break;
    case ColliderType::PLAYER:

        break;
    }
}
void DungBeetleBall::Bounce(const b2Vec2& normal)
{
    b2Vec2 velocity = pbody->body->GetLinearVelocity();

    // Reflejar velocidad con respecto a la normal
    b2Vec2 reflected = velocity - 2.0f * b2Dot(velocity, normal) * normal;

    // Normalizar y mantener la misma velocidad
    float speed = velocity.Length();
    reflected.Normalize();
    reflected *= speed;

    pbody->body->SetLinearVelocity(reflected);
}

