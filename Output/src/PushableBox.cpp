#include "PushableBox.h"
#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Physics.h"
#include "Log.h"

PushableBox::PushableBox() : Entity(EntityType::PUSHABLE_BOX) {}

PushableBox::~PushableBox() {}

bool PushableBox::Awake() { return true; }

bool PushableBox::Start()
{
    // Crear cuerpo físico dinámico y sin gravedad
    pbody = Engine::GetInstance().physics->CreateRectangle(position.getX() + texW / 2, position.getY() + texH / 2, texW, texH, DYNAMIC);
    pbody->ctype = ColliderType::WALL; // O uno nuevo, como PUSHABLE, si lo defines
    pbody->listener = this;

    pbody->body->SetGravityScale(0.0f);
    pbody->body->GetFixtureList()->SetFriction(2.0f); // Alta fricción
    pbody->body->GetFixtureList()->SetDensity(5.0f);  // Más masa
    pbody->body->ResetMassData();

    return true;
}

bool PushableBox::Update(float dt)
{
    b2Vec2 velocity = pbody->body->GetLinearVelocity();
    
    // Actualizar posición lógica desde física
    if (pbody && pbody->body)
    {
        b2Vec2 pos = pbody->body->GetPosition();
        position.x = METERS_TO_PIXELS(pos.x) - texW / 2;
        position.y = METERS_TO_PIXELS(pos.y) - texH / 2;
    }

    if (!touchingPlayer)
    {
        velocity.x = 0; 
        pbody->body->SetLinearVelocity(velocity);
    }

    // Dibujar textura
    Engine::GetInstance().render->DrawTexture(texture, position.getX(), position.getY());
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

    std::string texturePath = parameters.attribute("texture").as_string();
    texture = Engine::GetInstance().textures->Load(texturePath.c_str());
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
