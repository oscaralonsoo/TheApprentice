#include "PushableBox.h"
#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Physics.h"
#include "Log.h"
#include "Player.h"
#include "Scene.h"

PushableBox::PushableBox() : Entity(EntityType::PUSHABLE_BOX) {}

PushableBox::~PushableBox() {}

bool PushableBox::Awake() { return true; }

bool PushableBox::Start()
{
    // Crear cuerpo físico dinámico y sin gravedad
    pbody = Engine::GetInstance().physics->CreateRectangle(position.getX() + texW / 2, position.getY() + texH / 2, texW, texH, DYNAMIC, 7, 20,
        CATEGORY_BOX,
        CATEGORY_PLAYER | CATEGORY_PLATFORM | CATEGORY_BOX
    );
    pbody->ctype = ColliderType::WALL; // O uno nuevo, como PUSHABLE, si lo defines
    pbody->listener = this;

    pbody->body->SetGravityScale(5.0f);
    pbody->body->GetFixtureList()->SetFriction(0.0f); // Alta fricción
    pbody->body->GetFixtureList()->SetDensity(5.0f);  // Más masa
    pbody->body->ResetMassData();

    pbody->ctype = ColliderType::BOX;


    std::string path = "Assets/Props/box" + std::to_string(rand() % 3) + ".png";
    texture = Engine::GetInstance().textures->Load(path.c_str());

    return true;
}

bool PushableBox::Update(float dt)
{
    if (pbody && pbody->body)
    {
        b2Vec2 pos = pbody->body->GetPosition();
        position.x = METERS_TO_PIXELS(pos.x) - texW / 2;
        position.y = METERS_TO_PIXELS(pos.y) - texH / 2;
    }

    Player* player = Engine::GetInstance().scene->GetPlayer();

    if (player && !player->GetMechanics()->CanPush())
    {
        if (!isStatic)
        {
            RecreateBody(STATIC); // Usa tu enum, no el de Box2D
            isStatic = true;
        }
    }
    else
    {
        if (isStatic)
        {
            RecreateBody(DYNAMIC);
            isStatic = false;
        }

        if (!touchingPlayer && pbody)
        {
            b2Vec2 vel = pbody->body->GetLinearVelocity();
            vel.x = 0.0f;
            pbody->body->SetLinearVelocity(vel);
        }
    }

    if (player && player->GetMechanics()->CanPush() && touchingPlayer)
    {
        player->SetState("push");
    }


    Engine::GetInstance().render->DrawTexture(texture, (int)position.getX(), (int)position.getY());

    return true;
}

bool PushableBox::CleanUp()
{
    if (pbody)
        Engine::GetInstance().physics->DeletePhysBody(pbody);
    return true;
}

void PushableBox::SetParameters(pugi::xml_node parameters)
{
    position.x = parameters.attribute("x").as_int();
    position.y = parameters.attribute("y").as_int();
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();
}

void PushableBox::SetPosition(Vector2D pos)
{
    position = pos;
    if (pbody && pbody->body)
    {
        b2Vec2 bodyPos(PIXEL_TO_METERS(pos.getX() + texW / 2), PIXEL_TO_METERS(pos.getY() + texH / 2));
        pbody->body->SetTransform(bodyPos, 0);
    }
}

Vector2D PushableBox::GetPosition() const
{
    return position;
}

void PushableBox::OnCollision(PhysBody* physA, PhysBody* physB)
{
    if (physB->ctype == ColliderType::PLAYER)
    {
        touchingPlayer = true;
    }
}

void PushableBox::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    if (physB->ctype == ColliderType::PLAYER)
    {
        touchingPlayer = false;
    }
}

void PushableBox::RecreateBody(bodyType type)
{
    if (pbody)
    {
        Engine::GetInstance().physics->DeletePhysBody(pbody);
        pbody = nullptr;
    }

    pbody = Engine::GetInstance().physics->CreateRectangle(
        position.getX() + texW / 2,
        position.getY() + texH / 2,
        texW, texH,
        type,
        CATEGORY_BOX,
        CATEGORY_PLAYER | CATEGORY_PLATFORM | CATEGORY_BOX
    );

    pbody->ctype = ColliderType::BOX;
    pbody->listener = this;

    pbody->body->SetGravityScale(5.0f);
    pbody->body->GetFixtureList()->SetDensity(5.0f);
    pbody->body->ResetMassData();
}