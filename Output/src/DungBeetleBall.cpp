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
        }
    }

    pbody = Engine::GetInstance().physics->CreateCircle(x, y, width / 2, bodyType::DYNAMIC);
    pbody->ctype = ColliderType::ENEMY;
    pbody->listener = this;

    pbody->body->SetGravityScale(0.0f);
    if (b2Fixture* fixture = pbody->body->GetFixtureList()) {
        fixture->SetSensor(false);

        b2Filter filter;
        filter.categoryBits = CATEGORY_ENEMY;
        filter.maskBits = CATEGORY_WALL | CATEGORY_PLAYER_DAMAGE | CATEGORY_PLATFORM | CATEGORY_ATTACK | CATEGORY_PLAYER;
        fixture->SetFilterData(filter);

        // Configuración mejorada para rebotes
        fixture->SetRestitution(1.0f); 
        fixture->SetFriction(0.0f);     
        fixture->SetDensity(1.0f);     

        pbody->body->SetLinearDamping(0.0f);   
        pbody->body->SetAngularDamping(0.0f);   
        pbody->body->SetBullet(true);          
    }

    pbody->body->SetLinearVelocity(b2Vec2(direction.x * speed, direction.y * speed));

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
    Engine::GetInstance().render->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame() );
    currentAnimation->Update();

    b2Vec2 currentPos = pbody->body->GetPosition();

    if (b2DistanceSquared(previousPosition, currentPos) < 0.01f * 0.01f)
    {
        timeStuck += dt;
        if (timeStuck > 1000.0f) 
        {
            // Aplicar pequeño impulso aleatorio para liberarla
            b2Vec2 randomDir((rand() % 100 - 50) / 100.0f, (rand() % 100 - 50) / 100.0f);
            randomDir.Normalize();
            pbody->body->ApplyLinearImpulse(0.5f * randomDir, pbody->body->GetWorldCenter(), true);
            timeStuck = 0.0f; // Resetear contador
        }
    }
    else
        timeStuck = 0.0f;

    pbody->body->SetAngularVelocity(0.0f);

    b2Vec2 velocity = pbody->body->GetLinearVelocity();
    if (fabs(velocity.Length() - speed) > 0.1f)
    {
        velocity.Normalize();
        velocity *= speed;
        pbody->body->SetLinearVelocity(velocity);
    }
    previousPosition = currentPos;
    time += dt;

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
    case ColliderType ::ATTACK:
    case ColliderType::PLATFORM:
    case ColliderType::WALL:
        Bounce();
        break;
    case ColliderType::PLAYER:
        // TODO TONI --- DETECTAR COLISION!
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

    if (velocity.Length() > 0) {
        velocity.Normalize();

        float randomAngle = ((rand() % 100) / 100.0f - 0.5f) * 0.4f;
        float angle = atan2(velocity.y, velocity.x) + randomAngle;

        b2Vec2 newVelocity(cosf(angle), sinf(angle));
        newVelocity *= speed;
        pbody->body->SetLinearVelocity(newVelocity);
    }

    pbody->body->SetAngularVelocity(0.0f);
}



