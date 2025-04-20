#include "DustParticle.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "ParticleManager.h"
#include "Textures.h"


DustParticle::DustParticle() : Entity(EntityType::DUST_PARTICLE), state(DustParticleState::SPAWNING)
{
}

DustParticle::~DustParticle() {
}

bool DustParticle::Awake() {
    return true;
}

bool DustParticle::Start() {
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    pugi::xml_node dustNode = loadFile.child("config").child("scene").child("animations").child("particles").child("dust");

    // Cargar textura y animaciones
    texture = Engine::GetInstance().textures->Load(dustNode.attribute("texture").as_string());
    spawnAnim.LoadAnimations(dustNode.child("spawn"));
    floatAnim.LoadAnimations(dustNode.child("float"));
    fadeAnim.LoadAnimations(dustNode.child("fade"));

    scale = 0.3f + static_cast<float>(rand()) / RAND_MAX * (1.0f - 0.3f);

    // Aplicar escala al tamaño
    texW = dustNode.attribute("w").as_int() * scale;
    texH = dustNode.attribute("h").as_int() * scale;

    // Crear cuerpo físico con el tamaño escalado
    pbody = Engine::GetInstance().physics->CreateRectangleSensor(
        (int)position.getX() + texW / 2,
        (int)position.getY() + texH / 2,
        texW, texH,
        bodyType::DYNAMIC,
        CATEGORY_DUST_PARTICLE,  // su propia categoría
        0x0000                   // no colisiona con nada
    );

    pbody->body->SetGravityScale(0);

    currentAnimation = &spawnAnim;

    // Movimiento aleatorio
    float angle = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;
    float speed = 0.25f;

    float vx = cos(angle) * speed;
    float vy = sin(angle) * speed;

    pbody->body->SetLinearVelocity(b2Vec2(vx, vy));

    pbody->body->SetLinearVelocity(b2Vec2(vx, vy));

    return true;
}

bool DustParticle::Update(float dt) {

    switch (state)
    {
    case DustParticleState::SPAWNING:
        if (currentAnimation != &spawnAnim) currentAnimation = &spawnAnim;
        if (currentAnimation->HasFinished()) state = DustParticleState::FLOATING;
        break;
    case DustParticleState::FLOATING:
        if (currentAnimation != &floatAnim) currentAnimation = &floatAnim;
        if (floatTimer.ReadMSec() == 0) floatTimer.Start();

        if (floatTimer.ReadMSec() > 2500) state = DustParticleState::FADING;
        
        break;
    case DustParticleState::FADING:
        if (currentAnimation != &fadeAnim) currentAnimation = &fadeAnim;

        break;
    }

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_NONE, scale);
    currentAnimation->Update();

    return true;
}

bool DustParticle::PostUpdate() {
    if (currentAnimation->HasFinished() && state == DustParticleState::FADING) Engine::GetInstance().particleManager.get()->DestroyParticle(this);

    return true;
}

bool DustParticle::CleanUp() {
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    return true;
}

void DustParticle::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}