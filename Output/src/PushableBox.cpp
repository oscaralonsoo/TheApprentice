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
    pbody = Engine::GetInstance().physics->CreateRectangle((int)position.getX() + width / 2, (int)position.getY() + height / 2, width, height, DYNAMIC, 0, 0,
        CATEGORY_BOX,
        CATEGORY_PLAYER | CATEGORY_PLATFORM | CATEGORY_BOX | CATEGORY_ENEMY | CATEGORY_DOOR
    );
    pbody->listener = this;

    pbody->body->SetGravityScale(5.0f);
    pbody->body->GetFixtureList()->SetFriction(0.0f);
    pbody->body->GetFixtureList()->SetDensity(4.0f);  
    pbody->body->ResetMassData();

    pbody->ctype = ColliderType::BOX;


    std::string path = "Assets/Props/box" + std::to_string(rand() % 3) + ".png";
    texture = Engine::GetInstance().textures->Load(path.c_str());

    return true;
}

bool PushableBox::Update(float dt)
{
    Player* player = Engine::GetInstance().scene->GetPlayer();

    if (!player->GetMechanics()->CanPush() && isPlayerPushing) {
        pbody->body->SetType(b2_kinematicBody);
        pbody->body->SetLinearVelocity({ 0, 0 });
    }
    else {
        pbody->body->SetType(b2_dynamicBody);
        if (!isPlayerPushing && !isEnemyPushing) {
            pbody->body->SetLinearVelocity({ 0, pbody->body->GetLinearVelocity().y });
        }
        if (player->GetMechanics()->CanPush() && isPlayerPushing) {

            if (fabs(pbody->body->GetLinearVelocity().x) > 0.01f)
            {
                player->SetState("push");
            }
        }
    }

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - width / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - height / 2);

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY());

    return true;
}

bool PushableBox::CleanUp()
{
    if (pbody)
        Engine::GetInstance().physics->DeletePhysBody(pbody);
    return true;
}

void PushableBox::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + width / 2);
    pos.setY(pos.getY() + height / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D PushableBox::GetPosition() const
{
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}

void PushableBox::OnCollision(PhysBody* physA, PhysBody* physB)
{
    if (physB->ctype == ColliderType::PLAYER)
    {
        isPlayerPushing = true;
    }
    if (physB->ctype == ColliderType::ENEMY)
    {
        isEnemyPushing = true;
    }

}

void PushableBox::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    if (physB->ctype == ColliderType::PLAYER)
    {
        isPlayerPushing = false;
        pbody->body->SetLinearVelocity({ 0, pbody->body->GetLinearVelocity().y });

    }
    if (physB->ctype == ColliderType::ENEMY)
    {
        isEnemyPushing = false;
        pbody->body->SetLinearVelocity({ 0, pbody->body->GetLinearVelocity().y });
    }

}