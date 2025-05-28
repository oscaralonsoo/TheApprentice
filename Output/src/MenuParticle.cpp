#include "MenuParticle.h"
#include "Engine.h"
#include "Scene.h"
#include "ParticleManager.h"
#include "Textures.h"
#include <cmath>

MenuParticle::MenuParticle() : Entity(EntityType::MENU_PARTICLE), state(MenuParticleState::SPAWNING)
{
    velocity = { 0.0f, 0.0f };
}

MenuParticle::~MenuParticle() {
}

bool MenuParticle::Awake() {
    return true;
}

bool MenuParticle::Start() {
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

bool MenuParticle::Update() {
    switch (state)
    {
    case MenuParticleState::SPAWNING:
        if (currentAnimation != &spawnAnim) currentAnimation = &spawnAnim;
        if (currentAnimation->HasFinished()) state = MenuParticleState::FLOATING;
        break;
    case MenuParticleState::FLOATING:
        if (currentAnimation != &floatAnim) currentAnimation = &floatAnim;
        if (floatTimer.ReadMSec() == 0) floatTimer.Start();
        if (floatTimer.ReadMSec() > 2500) state = MenuParticleState::FADING;
        break;
    case MenuParticleState::FADING:
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

bool MenuParticle::PostUpdate() {
    if (currentAnimation->HasFinished() && state == MenuParticleState::FADING)
        Engine::GetInstance().menus->DestroyMenuParticle(this);

    return true;
}

bool MenuParticle::CleanUp() {
    return true;
}

void MenuParticle::SetPosition(Vector2D pos) {
    position.setX(pos.getX());
    position.setY(pos.getY());
}
