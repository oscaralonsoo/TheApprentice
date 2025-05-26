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
    // Crear cuerpo f�sico din�mico y sin gravedad
    pbody = Engine::GetInstance().physics->CreateRectangle(position.getX(), position.getY()+3, texW, texH, DYNAMIC, 0, 0,
        CATEGORY_BOX,
        CATEGORY_PLAYER | CATEGORY_PLATFORM | CATEGORY_BOX
    );
    pbody->ctype = ColliderType::WALL; // O uno nuevo, como PUSHABLE, si lo defines
    pbody->listener = this;

    pbody->body->SetGravityScale(5.0f);
    pbody->body->GetFixtureList()->SetFriction(0.0f); // Alta fricci�n
    pbody->body->GetFixtureList()->SetDensity(5.0f);  // M�s masa
    pbody->body->ResetMassData();

    pbody->ctype = ColliderType::BOX;


    std::string path = "Assets/Props/box" + std::to_string(rand() % 3) + ".png";
    texture = Engine::GetInstance().textures->Load(path.c_str());

    return true;
}

bool PushableBox::Update(float dt)
{
    if (!canEnemyPush && pushEnemy.ReadMSec() >= pushEnemyTimer)
    {
        canEnemyPush = true;
    }

    if (!canPlatformPush && pushEnemy.ReadMSec() >= pushPlatformTimer)
    {
        canPlatformPush = true;
    }

    if (pbody && pbody->body)
    {
        b2Vec2 pos = pbody->body->GetPosition();
        position.x = METERS_TO_PIXELS(pos.x) - texW / 2;
        position.y = METERS_TO_PIXELS(pos.y) - texH / 2;
    }

    Player* player = Engine::GetInstance().scene->GetPlayer();

    bool playerCanPush = player && player->GetMechanics()->CanPush();
    bool someoneCanPush = playerCanPush || touchingEnemy;

    if (!someoneCanPush && touchingPlatform)
    {
        if (!isStatic)
        {
            RecreateBody(STATIC);
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

        if (!touchingPlayer && !touchingEnemy && pbody)
        {
            b2Vec2 vel = pbody->body->GetLinearVelocity();
            vel.x = 0.0f;
            pbody->body->SetLinearVelocity(vel);
        }
    }

    if (playerCanPush && touchingPlayer)
    {
        player->SetState("push");
    }

    Engine::GetInstance().render->DrawTexture(texture, (int)position.getX() +20, (int)position.getY() +22);

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
    if (physB->ctype == ColliderType::ENEMY)
    {
        if (canEnemyPush)
        {
            touchingEnemy = true;
        }
    }
    if (physB->ctype == ColliderType::PLATFORM || physB->ctype == ColliderType::WALL)
    {
        if (canPlatformPush)
        {
            touchingPlatform = true;
        }
    }

}

void PushableBox::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    if (physB->ctype == ColliderType::PLAYER)
    {
        touchingPlayer = false;
    }
    if (physB->ctype == ColliderType::ENEMY)
    {
        touchingEnemy = false;
        canEnemyPush = false;
        pushEnemy.Start();
    }
    if (physB->ctype == ColliderType::PLATFORM || physB->ctype == ColliderType::WALL)
    {
        touchingPlatform = false;
        canPlatformPush = false;
        pushPlatform.Start();
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
        position.getX(),
        position.getY() + 40,
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

    // ✅ Añadimos el damping para frenar la caja de forma suave
    pbody->body->SetLinearDamping(10.0f);  // Puedes ajustar este valor

    if (type == DYNAMIC) {
        // Obtener la posición actual del body en metros
        b2Vec2 currentPos = pbody->body->GetPosition();

        // Moverlo un poco a la derecha (por ejemplo, 0.1 metros)
        b2Vec2 newPos = b2Vec2(currentPos.x + 0.65f, currentPos.y - 0.15);

        // Aplicar la nueva posición
        pbody->body->SetTransform(newPos, 0);
    }
}