#include "DustParticle.h"
#include "Engine.h"
#include "Scene.h"
#include "ParticleManager.h"
#include "Textures.h"
#include <cmath>

DustParticle::DustParticle() : Entity(EntityType::DUST_PARTICLE), state(DustParticleState::SPAWNING)
{
    velocity = { 0.0f, 0.0f };
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

    texW = dustNode.attribute("w").as_int() * scale;
    texH = dustNode.attribute("h").as_int() * scale;

    // Movimiento aleatorio
    float angle = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;
    float speed = 0.25f;
    velocity = {
        cos(angle) * speed,
        sin(angle) * speed
    };

    currentAnimation = &spawnAnim;

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

    // Actualizar posición con la velocidad
    position.setX(position.getX() + velocity.x);
    position.setY(position.getY() + velocity.y);

    Engine::GetInstance().render->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_NONE, scale);
    currentAnimation->Update();

    return true;
}

bool DustParticle::PostUpdate() {
    if (currentAnimation->HasFinished() && state == DustParticleState::FADING)
        Engine::GetInstance().particleManager->DestroyParticle(this);

    return true;
}

bool DustParticle::CleanUp() {
    return true; // Ya no hay pbody que destruir
}

void DustParticle::SetPosition(Vector2D pos) {
    position.setX(pos.getX());
    position.setY(pos.getY());
}
