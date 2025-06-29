#include "NullwardenSpear.h"
#include "Engine.h"
#include "EntityManager.h"
#include "Scene.h"

NullwardenSpear::NullwardenSpear(float x, float y, float speed, b2Vec2 direction)
    : Entity(EntityType::SPEAR), direction(direction)
{
    position = Vector2D(x, y);
    bool horizontal = (direction.x != 0.0f);

    width = horizontal ? 180 : 24;
    height = horizontal ? 24 : 180;

    pbody = Engine::GetInstance().physics->CreateRectangleSensor(position.x, position.y, width, height, bodyType::DYNAMIC, CATEGORY_ENEMY, CATEGORY_WALL | CATEGORY_PLAYER_DAMAGE);
    pbody->ctype = ColliderType::ENEMY;
    pbody->listener = this;

    pbody->body->SetGravityScale(0.0f);

    pbody->body->SetLinearVelocity(b2Vec2(direction.x * speed, direction.y * speed));

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node node = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); node; node = node.next_sibling("enemy"))
    {
        if (std::string(node.attribute("type").as_string()) == "NullwardenSpear")
        {
            texture = Engine::GetInstance().textures.get()->Load(node.attribute("texture").as_string());

            spawnAnim.LoadAnimations(node.child("spawn"));
            idleAnim.LoadAnimations(node.child("idle"));
            destroyedAnim.LoadAnimations(node.child("destroyed"));
        }
    }

    currentAnimation = &idleAnim;
}

NullwardenSpear::~NullwardenSpear() {
    if (pbody) {
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        pbody = nullptr;
    }
    delete currentAnimation;
    currentAnimation = nullptr;
}

bool NullwardenSpear::Update(float dt) {
    b2Transform pbodyPos = pbody->body->GetTransform();

    bool horizontal = (direction.x != 0.0f);

    if (horizontal) {
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - height / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - width / 2);
    }
    else {
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - width / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - height / 2);
    }

    float angle = horizontal ? 90.0f : 0.0f;
    SDL_RendererFlip flip = (direction.x < 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    Engine::GetInstance().render.get()->DrawTexture(
        texture,
        (int)position.getX(),
        (int)position.getY(),
        &currentAnimation->GetCurrentFrame(),
        1.0f,
        angle,
        INT_MAX,
        INT_MAX,
        flip
    );

    currentAnimation->Update();

    return true;
}

bool NullwardenSpear::CleanUp()
{
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    return true;
}

void NullwardenSpear::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::WALL:
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    }
}

void NullwardenSpear::Activate()
{
}

void NullwardenSpear::Deactivate()
{
}
