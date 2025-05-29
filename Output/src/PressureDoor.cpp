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
        CATEGORY_PLATFORM,
        CATEGORY_PLAYER
    );

    pbody->ctype = ColliderType::PLATFORM;
    pbody->listener = this;
    pbody->body->SetGravityScale(0);

    if (width > height) isHorizontal = true;

    currentAnimation = &idleAnim;

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
            pbody->body->GetFixtureList()->SetSensor(false);
            Vector2D positionWorld = Engine::GetInstance().map->WorldToMap(position.x, position.y);
            Engine::GetInstance().map.get()->SetNavigationTileRegion(positionWorld.x, positionWorld.y, width / 64, height / 64, 1);
        }
        if (currentAnimation->HasFinished()) state = PressureDoorState::IDLE;

        break;
    case PressureDoorState::IDLE:
        if (currentAnimation != &idleAnim) currentAnimation = &idleAnim;
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
        y -= height + 25;
    }
    else {
        x -= width + 25;
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
    else
        state = PressureDoorState::ENABLE;

    if (pbody && pbody->body && pbody->body->GetFixtureList())
        pbody->body->GetFixtureList()->SetSensor(value);
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