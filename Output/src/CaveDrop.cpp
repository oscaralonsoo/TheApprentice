#include "CaveDrop.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

CaveDrop::CaveDrop() : Entity(EntityType::CAVEDROP), state(CaveDropStates::START)
{
    name = "CaveDrop";
}

CaveDrop::~CaveDrop() {}

bool CaveDrop::Awake() {
    return true;
}

bool CaveDrop::Start() {
    // Cargar configuraciones desde XML
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    pugi::xml_node caveDropNode = loadFile.child("config").child("scene").child("animations").child("props").child("cave_drop");

    // Cargar textura y animaciones
    texture = Engine::GetInstance().textures->Load(caveDropNode.attribute("texture").as_string());
    startAnim.LoadAnimations(caveDropNode.child("start"));
    //fallAnim.LoadAnimations(caveDropNode.child("fall"));
    //splashAnim.LoadAnimations(caveDropNode.child("splash"));

    texW = caveDropNode.attribute("w").as_int();
    texH = caveDropNode.attribute("h").as_int();

    // Crear cuerpo físico
    pbody = Engine::GetInstance().physics->CreateRectangleSensor((int)position.getX(), (int)position.getY(), texW, texH, bodyType::DYNAMIC);
    pbody->ctype = ColliderType::CAVEDROP;
    pbody->listener = this;

    currentAnimation = &startAnim;
    return true;
}

bool CaveDrop::Update(float dt) {
    // Actualizar posición basada en el cuerpo físico
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    // Manejo de estados
    switch (state) {
    case CaveDropStates::START:
        if (currentAnimation->HasFinished()) {
            ChangeState(CaveDropStates::FALL);
        }
        break;

    case CaveDropStates::FALL:
        // Aquí podrías añadir condiciones como colisión con el suelo
        if (HasHitGround()) {
            ChangeState(CaveDropStates::SPLASH);
        }
        break;

    case CaveDropStates::SPLASH:
        if (currentAnimation->HasFinished()) {
            MarkForDeletion(); // Si necesitas eliminar la gota después del splash
        }
        break;
    }

    // Dibujar animación actual
    Engine::GetInstance().render->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
    currentAnimation->Update();

    return true;
}

void CaveDrop::ChangeState(CaveDropStates newState) {
    state = newState;

    switch (state) {
    case CaveDropStates::START:
        currentAnimation = &startAnim;
        break;
    case CaveDropStates::FALL:
        currentAnimation = &fallAnim;
        break;
    case CaveDropStates::SPLASH:
        currentAnimation = &splashAnim;
        break;
    }
    currentAnimation->Reset();
}

bool CaveDrop::HasHitGround() {
    return position.getY();
}

void CaveDrop::MarkForDeletion() {
    LOG("CaveDrop eliminado");
}

bool CaveDrop::CleanUp() {
    return true;
}

void CaveDrop::OnCollision(PhysBody* physA, PhysBody* physB) {
    printf("entra");
}

void CaveDrop::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {

}
