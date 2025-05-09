#include "PressureDoor.h"
#include "Engine.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

PressureDoor::PressureDoor() : Entity(EntityType::PRESSURE_PLATE), state(PressureDoorState::DISABLED)
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
    disabledAnim.LoadAnimations(node.child("disabled"));
    enabledAnim.LoadAnimations(node.child("enabled"));

    width = node.attribute("w").as_int();
    height = node.attribute("h").as_int();

    pbody = Engine::GetInstance().physics->CreateRectangle(
        (int)position.getX() + width / 2,
        (int)position.getY() + height / 2,
        width, height,
        STATIC,
        CATEGORY_PLATFORM,
        CATEGORY_PLAYER
    );

    pbody->ctype = ColliderType::WALL;
    pbody->listener = this;
    pbody->body->SetGravityScale(0);

    currentAnimation = &disabledAnim;
    return true;
}

bool PressureDoor::Update(float dt)
{
    switch (state) {
    case PressureDoorState::DISABLED:
        if (currentAnimation != &disabledAnim) currentAnimation = &disabledAnim;
        break;
    case PressureDoorState::ENABLED:
        if (currentAnimation != &enabledAnim) currentAnimation = &enabledAnim;

        break;
    }

    RenderTexture();

    return true;
}

bool PressureDoor::PostUpdate()
{
    if (isOpen) Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    
	return true;
}

bool PressureDoor::CleanUp()
{
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void PressureDoor::SetOpen(bool value) {
    isOpen = value;
}


void PressureDoor::RenderTexture() {
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - (texW / 2));
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - (texH / 2));

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + width / 2 - texW / 2, (int)position.getY() + height - texH, &currentAnimation->GetCurrentFrame());
    currentAnimation->Update();
}
