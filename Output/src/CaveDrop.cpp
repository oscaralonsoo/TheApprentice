#include "CaveDrop.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include <cstdlib>
#include <ctime>

CaveDrop::CaveDrop() : Entity(EntityType::CAVE_DROP), state(CaveDropStates::DISABLED)
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
    fallAnim.LoadAnimations(caveDropNode.child("fall"));
    splashAnim.LoadAnimations(caveDropNode.child("splash"));

    texW = caveDropNode.attribute("w").as_int();
    texH = caveDropNode.attribute("h").as_int();

    // Crear cuerpo físico
    pbody = Engine::GetInstance().physics->CreateRectangle((int)position.getX() + texW/2, (int)position.getY()+texH/2, 1, 1, bodyType::DYNAMIC);
    pbody->ctype = ColliderType::CAVE_DROP;
    pbody->listener = this;
    pbody->body->SetGravityScale(0);

    currentAnimation = &startAnim;
    initPos = {position.getX() + texW/2, position.getY() + texH/2 };

    randomTime = (std::rand() % MAX_RANDOM_TIME * 1000) + MIN_RANDOM_TIME * 1000;
    dropTimer.Start();

    return true;
}

bool CaveDrop::Update(float dt) {
    b2Vec2 velocity = b2Vec2(pbody->body->GetLinearVelocity());

    switch (state) {
    case CaveDropStates::DISABLED:
        currentAnimation = nullptr;

        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);

        if (dropTimer.ReadMSec() >= randomTime || dropTimer.ReadMSec() == 0) {
            state = CaveDropStates::START;
            dropTimer.Start();
        }
        break;
    case CaveDropStates::START:
        if (currentAnimation != &startAnim) {
            currentAnimation = &startAnim;
            currentAnimation->Reset();
        }
        if (position != initPos) pbody->body->SetTransform(b2Vec2(initPos.x / PIXELS_PER_METER, initPos.y / PIXELS_PER_METER), 0);
        if (currentAnimation->HasFinished()) state = CaveDropStates::FALL;
        break;

    case CaveDropStates::FALL:
        if (currentAnimation != &fallAnim) {
            currentAnimation = &fallAnim;
            currentAnimation->Reset();
        }
        if (velocity.y == 0) pbody->body->ApplyForceToCenter(b2Vec2(0, 0.1f), true);
        pbody->body->SetGravityScale(2.0f);
        break;

    case CaveDropStates::SPLASH:
        if (currentAnimation != &splashAnim) {
            currentAnimation = &splashAnim;
            currentAnimation->Reset();
        }

        pbody->body->SetGravityScale(0);
        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);

        if (currentAnimation->HasFinished()) {
            state = CaveDropStates::DISABLED;

            randomTime = (std::rand() % MAX_RANDOM_TIME * 1000) + MIN_RANDOM_TIME * 1000;
            dropTimer.Start();
        }
            
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

bool CaveDrop::CleanUp() {
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    return true;
}

void CaveDrop::OnCollision(PhysBody* physA, PhysBody* physB) {
    if (state == CaveDropStates::FALL) state = CaveDropStates::SPLASH;
}

void CaveDrop::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
}
