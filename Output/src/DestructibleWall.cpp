#include "DestructibleWall.h"
#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Physics.h"
#include "Log.h"
#include "Enemy.h"
#include "Bloodrusher.h"

DestructibleWall::DestructibleWall() : Entity(EntityType::DESTRUCTIBLE_WALL) {}

DestructibleWall::~DestructibleWall() {}

bool DestructibleWall::Awake() { return true; }

bool DestructibleWall::Start()
{
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    pugi::xml_node destructibleWallNode = loadFile.child("config").child("scene").child("animations").child("props").child("destructible_wall");

    // Cargar textura y animaciones
    texture = Engine::GetInstance().textures->Load(destructibleWallNode.attribute("texture").as_string());
    // Inicializar cuerpo físico como estático
    pbody = Engine::GetInstance().physics->CreateRectangle(position.getX() + texW / 2, position.getY() + texH / 2, texW, texH, STATIC);
    pbody->ctype = ColliderType::DESTRUCTIBLE_WALL;
    pbody->listener = this;


    return true;
}

bool DestructibleWall::Update(float dt)
{
    if (!destroyed)
    {
        Engine::GetInstance().render->DrawTexture(texture, position.getX(), position.getY());
    }
    return true;
}

bool DestructibleWall::CleanUp()
{
    if (pbody)
        Engine::GetInstance().physics->DeletePhysBody(pbody);

    return true;
}

void DestructibleWall::SetParameters(pugi::xml_node parameters)
{
    position.x = parameters.attribute("x").as_int();
    position.y = parameters.attribute("y").as_int();
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

    std::string texturePath = parameters.attribute("texture").as_string();
    texture = Engine::GetInstance().textures->Load(texturePath.c_str());
}

void DestructibleWall::SetPosition(Vector2D pos)
{
    position = pos;
    b2Vec2 bodyPos(PIXEL_TO_METERS(pos.getX() + texW / 2), PIXEL_TO_METERS(pos.getY() + texH / 2));
    if (pbody && pbody->body)
        pbody->body->SetTransform(bodyPos, 0);
}

Vector2D DestructibleWall::GetPosition() const
{
    return position;
}

void DestructibleWall::OnCollision(PhysBody* physA, PhysBody* physB)
{
    if (physB->ctype == ColliderType::ENEMY && !destroyed)
    {
        Entity* otherEntity = physB->listener;
        if (otherEntity != nullptr)
        {
            Bloodrusher* bloodrusher = dynamic_cast<Bloodrusher*>(otherEntity);
            if (bloodrusher && bloodrusher->GetCurrentState() == BloodrusherState::ATTACKING || bloodrusher->GetCurrentState() == BloodrusherState::DEAD)
            {
                LOG("Bloodrusher en estampida rompió la pared. Cambiando a SLIDING.");
                destroyed = true;

                Engine::GetInstance().physics->DeletePhysBody(pbody);
                pbody = nullptr;

                bloodrusher->SetState(BloodrusherState::SLIDING);
            }
        }
    }
}
