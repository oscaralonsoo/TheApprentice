#include "PressureDoor.h"
#include "Engine.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

PressureDoor::PressureDoor() : Entity(EntityType::PRESSURE_PLATE), state(PressureDoorState::DISABLE)
{
    name = "PressureDoor";
}

PressureDoor::~PressureDoor()
{
}

bool PressureDoor::Awake()
{
    return false;
}

bool PressureDoor::Start()
{
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    pugi::xml_node node = loadFile.child("config").child("scene").child("animations").child("props").child("pressure_door");

    texture = Engine::GetInstance().textures->Load(node.attribute("texture").as_string());
    disabledAnim.LoadAnimations(node.child("disable"));
    enabledAnim.LoadAnimations(node.child("enable"));
    idleAnim.LoadAnimations(node.child("idle"));

    pbody = Engine::GetInstance().physics->CreateRectangle(
        (int)position.getX() + width / 2,
        (int)position.getY() + height / 2,
        width, height,
        STATIC,
        0,
        0,
        CATEGORY_DOOR,
        CATEGORY_PLAYER | CATEGORY_BOX | CATEGORY_ENEMY
    );

    pbody->listener = this;
    pbody->ctype = ColliderType::BOX;
    pbody->body->SetGravityScale(0);

    if (width > height) isHorizontal = true;

    currentAnimation = &disabledAnim;
    CheckStartState();

    return true;
}


bool PressureDoor::Update(float dt)
{
    switch (state) {
    case PressureDoorState::DISABLE:
        if (currentAnimation != &disabledAnim) {
            currentAnimation = &disabledAnim;
            currentAnimation->Reset();
            pbody->body->GetFixtureList()->SetSensor(true);

            Vector2D positionWorld = Engine::GetInstance().map->WorldToMap(position.x, position.y);
            Engine::GetInstance().map.get()->SetNavigationTileRegion(positionWorld.x, positionWorld.y, width / 64, height / 64, 0);
        }

        if (!isOpen) state = PressureDoorState::ENABLE;

        break;
    case PressureDoorState::ENABLE:
        if (currentAnimation != &enabledAnim) {
            currentAnimation = &enabledAnim;
            currentAnimation->Reset();
            Vector2D positionWorld = Engine::GetInstance().map->WorldToMap(position.x, position.y);
            Engine::GetInstance().map.get()->SetNavigationTileRegion(positionWorld.x, positionWorld.y, width / 64, height / 64, 1);
        }
        if (currentAnimation->HasFinished()) state = PressureDoorState::IDLE;

        break;
    case PressureDoorState::IDLE:
        if (currentAnimation != &idleAnim) currentAnimation = &idleAnim;
        if (!IsOverlapping())
        {
            pbody->body->GetFixtureList()->SetSensor(false);
        }
        if (isOpen) state = PressureDoorState::DISABLE;
        break;
    }

    RenderTexture();
    return true;
}

bool PressureDoor::PostUpdate()
{
    return true;
}

bool PressureDoor::CleanUp()
{
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    return true;
}

void PressureDoor::RenderTexture()
{
    b2Transform pbodyPos = pbody->body->GetTransform();

    float x = METERS_TO_PIXELS(pbodyPos.p.x);
    float y = METERS_TO_PIXELS(pbodyPos.p.y);
    float angle = 0.0f;

    if (isHorizontal) {
        angle = 90.0f;
        x -= width / 2;
        y -= height + 30;
    }
    else {
        x -= width + 30;
        y -= height / 2;
    }

    Engine::GetInstance().render.get()->DrawTexture(
        texture,
        (int)x ,
        (int)y,
        &currentAnimation->GetCurrentFrame(),
        1.0f,
        angle
    );

    currentAnimation->Update();
}

void PressureDoor::SetOpen(bool value) {
    isOpen = value;
    if (value)
        state = PressureDoorState::DISABLE;
    else {
        if (state != PressureDoorState::IDLE) state = PressureDoorState::ENABLE;
    }
}
void PressureDoor::CheckStartState() {
    
    if (shouldBeOpen) {
        SetOpen(true);
        state = PressureDoorState::DISABLE;
        pbody->body->GetFixtureList()->SetSensor(true);
    }
    else {
        state = PressureDoorState::ENABLE;

    }
}

void PressureDoor::OnCollision(PhysBody* physA, PhysBody* physB)
{
}

void PressureDoor::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
}

bool PressureDoor::IsOverlapping() {
    for (b2ContactEdge* edge = pbody->body->GetContactList(); edge; edge = edge->next) {
        b2Contact* contact = edge->contact;
        if (contact->IsTouching()) {
            return true;
        }
    }
    return false;
}