#include "LifePlantMax.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

LifePlantMax::LifePlantMax() : Entity(EntityType::LIFE_PLANT_MAX), state(LifePlantMaxStates::AVAILABLE)
{
    name = "LifePlantMax";
}

LifePlantMax::~LifePlantMax() {}

bool LifePlantMax::Awake() {
    return true;
}

bool LifePlantMax::Start() {
    // Cargar configuraciones desde XML
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    pugi::xml_node lifePlantNode = loadFile.child("config").child("scene").child("animations").child("props").child("life_plant_max");

    // Cargar textura y animaciones
    texture = Engine::GetInstance().textures->Load(lifePlantNode.attribute("texture").as_string());
    availableAnim.LoadAnimations(lifePlantNode.child("available"));
    consumedAnim.LoadAnimations(lifePlantNode.child("consumed"));

    texW = lifePlantNode.attribute("w").as_int();
    texH = lifePlantNode.attribute("h").as_int();

    // Crear cuerpo físico
    pbody = Engine::GetInstance().physics->CreateRectangle(
        (int)position.getX() + texW / 2,
        (int)position.getY() + texH / 2,
        texW, texH,
        bodyType::DYNAMIC,
        0,
        0,
        CATEGORY_LIFE_PLANT,
        CATEGORY_ATTACK
    );
    pbody->ctype = ColliderType::LIFE_PLANT;
    pbody->listener = this;
    pbody->body->SetGravityScale(0);

    currentAnimation = &availableAnim;

    soundInteractId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Props/lifeplant_active.ogg", 1.0f);

    return true;
}

bool LifePlantMax::Update(float dt) {
    switch (state) {
    case LifePlantMaxStates::AVAILABLE:
        if (currentAnimation != &availableAnim) currentAnimation = &availableAnim;

        break;
    case LifePlantMaxStates::CONSUMED:

        if (!interactSoundPlayed) {
            Engine::GetInstance().audio->PlayFx(soundInteractId, 0.5f, 0);
            interactSoundPlayed = true;
        }

        if (currentAnimation != &consumedAnim) currentAnimation = &consumedAnim;
        break;
    }

    // Actualizar posición basada en el cuerpo físico
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    // Dibujar animación actual
    if (currentAnimation != nullptr) {
        Engine::GetInstance().render->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
        currentAnimation->Update();
    }

    return true;
}

bool LifePlantMax::CleanUp() {
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    return true;
}

void LifePlantMax::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
        if (state == LifePlantMaxStates::AVAILABLE)
        {
            state = LifePlantMaxStates::CONSUMED;
            Engine::GetInstance().scene->GetPlayer()->GetMechanics()->GetHealthSystem()->AddMaxLife();
            Engine::GetInstance().scene->TriggerVignetteFlash();
        }
        break;
    }
}

void LifePlantMax::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
}
