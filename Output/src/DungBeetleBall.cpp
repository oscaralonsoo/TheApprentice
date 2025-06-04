#include "DungBeetleBall.h"
#include "Engine.h"
#include "EntityManager.h"
#include "Scene.h"
DungBeetleBall::DungBeetleBall(float x, float y, float speed, b2Vec2 direction)
    : Entity(EntityType::BALL), direction(direction), speed(speed)
{
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node node = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); node; node = node.next_sibling("enemy"))
    {
        if (std::string(node.attribute("type").as_string()) == "DungBeetleBall")
        {
            texture = Engine::GetInstance().textures.get()->Load(node.attribute("texture").as_string());
            width = node.attribute("w").as_int();
            height = node.attribute("h").as_int();
            idleAnim.LoadAnimations(node.child("idle"));
            destroyAnim.LoadAnimations(node.child("destroyed"));
        }
    }

    pbody = Engine::GetInstance().physics->CreateCircleSensor(x, y, width / 2, bodyType::DYNAMIC, 2, 5);
    pbody->ctype = ColliderType::ENEMY;
    pbody->listener = this;

    pbody->body->SetGravityScale(0.0f);
    if (b2Fixture* fixture = pbody->body->GetFixtureList()) {

        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_WALL | CATEGORY_PLAYER_DAMAGE | CATEGORY_PLATFORM | CATEGORY_ATTACK | CATEGORY_PLAYER;
        fixture->SetFilterData(filter);

        fixture->SetRestitution(1.0f);
        fixture->SetFriction(0.0f);
        fixture->SetDensity(1.0f);

        pbody->body->SetLinearDamping(0.0f);
        pbody->body->SetAngularDamping(0.0f);
    }

    pbody->body->SetLinearVelocity(b2Vec2(direction.x * speed, direction.y * speed));

    currentAnimation = &idleAnim;
}

DungBeetleBall::~DungBeetleBall() {

}

bool DungBeetleBall::Update(float dt)
{
    CollisionNavigationLayer();

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - width / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - height / 2);
    Engine::GetInstance().render->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
    currentAnimation->Update();

    return true;
}
bool DungBeetleBall::PostUpdate() {
    if (currentAnimation == &destroyAnim) {
        pbody->body->GetFixtureList()->SetSensor(true);
        if (destroyAnim.HasFinished()) {
            Engine::GetInstance().entityManager->DestroyEntity(this);
        }
    }

    return true;
}

bool DungBeetleBall::CleanUp()
{
    Engine::GetInstance().physics->DeletePhysBody(pbody);
    return true;
}
void DungBeetleBall::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
    case ColliderType::PLAYER:
        Bounce();
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
void DungBeetleBall::Bounce()
{
    b2Vec2 velocity = pbody->body->GetLinearVelocity();
    if (velocity.LengthSquared() < 0.01f) return;

    velocity.Normalize();
    Vector2D mapPos = Engine::GetInstance().map->WorldToMap(
        METERS_TO_PIXELS(pbody->body->GetPosition().x),
        METERS_TO_PIXELS(pbody->body->GetPosition().y)
    );

    MapLayer* layer = Engine::GetInstance().map->GetNavigationLayer();
    b2Vec2 normal(0.0f, 0.0f);

    if (velocity.x < 0 && layer->Get(mapPos.x - 1, mapPos.y)) normal += b2Vec2(1, 0);
    else if (velocity.x > 0 && layer->Get(mapPos.x + 1, mapPos.y)) normal += b2Vec2(-1, 0);
    if (velocity.y < 0 && layer->Get(mapPos.x, mapPos.y - 1)) normal += b2Vec2(0, 1);
    else if (velocity.y > 0 && layer->Get(mapPos.x, mapPos.y + 1)) normal += b2Vec2(0, -1);

    b2Vec2 finalVel;

    if (normal.LengthSquared() > 0.0f)
    {
        normal.Normalize();
        b2Vec2 reflected = velocity - 2.0f * b2Dot(velocity, normal) * normal;

        float angle = atan2(reflected.y, reflected.x);
        angle += (((rand() % 100) / 100.0f - 0.5f) * 0.2f); // random ±0.1 radians
        finalVel.Set(cosf(angle), sinf(angle));
        finalVel *= speed;

        if (finalVel.LengthSquared() < 0.01f)
            finalVel = speed * normal;
    }
    else
    {
        finalVel = -speed * velocity;
    }

    pbody->body->SetLinearVelocity(finalVel);
    pbody->body->SetAngularVelocity(0.0f);
}


void DungBeetleBall::CollisionNavigationLayer() {
    b2Vec2 vel = pbody->body->GetLinearVelocity();
    if (vel.LengthSquared() < 0.01f) return;

    b2Vec2 pos = pbody->body->GetPosition();
    Vector2D projected = Engine::GetInstance().map->WorldToMap(
        METERS_TO_PIXELS(pos.x + vel.x * 2.0f),
        METERS_TO_PIXELS(pos.y + vel.y * 2.0f)
    );

    if (projected != currentTileMap)
    {
        if (Engine::GetInstance().map->GetNavigationLayer()->Get(projected.x, projected.y))
            Bounce();
        currentTileMap = projected;
    }
}