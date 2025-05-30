#include "PressurePlate.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

PressurePlate::PressurePlate() : Entity(EntityType::PRESSURE_PLATE), state(PressurePlateState::DISABLED)
{
    name = "PressurePlate";
}

PressurePlate::~PressurePlate()
{
}

bool PressurePlate::Awake()
{
	return false;
}

bool PressurePlate::Start()
{
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    pugi::xml_node node = loadFile.child("config").child("scene").child("animations").child("props").child("pressure_plate");

    texture = Engine::GetInstance().textures->Load(node.attribute("texture").as_string());
    disabledAnim.LoadAnimations(node.child("disabled"));
    enabledAnim.LoadAnimations(node.child("enabled"));

    width = 64;
    height = 64;

    pbody = Engine::GetInstance().physics->CreateRectangleSensor(
        (int)position.getX() + width / 2 ,
        (int)position.getY() + height / 2,
        width, height,
        DYNAMIC,
        CATEGORY_PRESSURE_PLATE,
        CATEGORY_PLATFORM | CATEGORY_BOX
    );

    pbody->ctype = ColliderType::PRESSURE_PLATE;
    pbody->listener = this;
    pbody->body->SetGravityScale(0);

    currentAnimation = &disabledAnim;
    return true;
}

bool PressurePlate::Update(float dt)
{
    switch (state) {
    case PressurePlateState::DISABLED:
        if (currentAnimation != &disabledAnim) currentAnimation = &disabledAnim;
        break;
    case PressurePlateState::ENABLED:
        if (currentAnimation != &enabledAnim) {
            currentAnimation = &enabledAnim;
            currentAnimation->Reset();
        }

        break;
    }

    RenderTexture();

    return true;
}

bool PressurePlate::PostUpdate()
{
	return true;
}

bool PressurePlate::CleanUp()
{
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void PressurePlate::OnCollision(PhysBody* physA, PhysBody* physB)
{
    if (physB->ctype == ColliderType::PLAYER || physB->ctype == ColliderType::BOX)
    {
        {
            state = PressurePlateState::ENABLED;
            SetActive(true);
        }
    }
}

void PressurePlate::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    if (physB->ctype == ColliderType::PLAYER || physB->ctype == ColliderType::BOX) {
        state = PressurePlateState::DISABLED;
        SetActive(false);
    }
}

void PressurePlate::RenderTexture() {
    if (isInvisible)
    {
        return;
    }

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x));
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y));

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - width * 1.5 , (int)position.getY() - height / 2, &currentAnimation->GetCurrentFrame());
    currentAnimation->Update();
}

void PressurePlate::SetActive(bool value)
{
    if (active != value)
    {
        active = value;
        Engine::GetInstance().pressureSystem->UpdateSystem();
    }
}
