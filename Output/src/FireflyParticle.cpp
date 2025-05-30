#include "FireflyParticle.h"
#include "Engine.h"
#include "Scene.h"
#include "ParticleManager.h"
#include "Textures.h"
#include <cmath>
#include <cstdlib>

FireflyParticle::FireflyParticle()
    : Entity(EntityType::PARTICLE), state(FireflyParticleState::SPAWNING)
{
    velocity = { 0.0f, 0.0f };
    directionAngle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
    angleChangeCooldown = 0.0f;
}

FireflyParticle::~FireflyParticle() {}

bool FireflyParticle::Awake() {
    return true;
}

bool FireflyParticle::Start() {
    pugi::xml_document loadFile;
    loadFile.load_file("config.xml");
    pugi::xml_node fireflyNode = loadFile.child("config").child("scene").child("animations").child("particles").child("firefly");

    texture = Engine::GetInstance().textures->Load(fireflyNode.attribute("texture").as_string());
    spawnAnim.LoadAnimations(fireflyNode.child("spawn"));
    floatAnim.LoadAnimations(fireflyNode.child("float"));
    fadeAnim.LoadAnimations(fireflyNode.child("fade"));

    scale = 0.7f + static_cast<float>(rand()) / RAND_MAX * (1.2f - 0.7f);

    texW = fireflyNode.attribute("w").as_int() * scale;
    texH = fireflyNode.attribute("h").as_int() * scale;

    currentAnimation = &spawnAnim;

    return true;
}

bool FireflyParticle::Update(float dt) {
    switch (state) {
    case FireflyParticleState::SPAWNING:
        if (currentAnimation != &spawnAnim) currentAnimation = &spawnAnim;
        if (currentAnimation->HasFinished()) state = FireflyParticleState::FLOATING;
        break;
    case FireflyParticleState::FLOATING:
        if (currentAnimation != &floatAnim) currentAnimation = &floatAnim;
        if (floatTimer.ReadMSec() == 0) floatTimer.Start();
        if (floatTimer.ReadMSec() > 4000) state = FireflyParticleState::FADING;
        break;
    case FireflyParticleState::FADING:
        if (currentAnimation != &fadeAnim) currentAnimation = &fadeAnim;
        break;
    }

    // Movimiento tipo luciérnaga
    angleChangeCooldown -= dt;
    if (angleChangeCooldown <= 0.0f) {
        // Variación aleatoria suave en el ángulo
        float angleVariation = ((rand() % 2001) - 1000) / 1000.0f * 0.3f; // ±0.3 rad aprox
        directionAngle += angleVariation;
        angleChangeCooldown = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.3f; // nuevo cooldown
    }

    float speed = 0.1f + static_cast<float>(rand()) / RAND_MAX * 0.05f; // pequeña variación
    velocity.x = cosf(directionAngle) * speed;
    velocity.y = sinf(directionAngle) * speed;

    position.setX(position.getX() + velocity.x);
    position.setY(position.getY() + velocity.y);

    Engine::GetInstance().render->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_NONE, SDL_FLIP_NONE, scale);
    currentAnimation->Update();

    return true;
}

bool FireflyParticle::PostUpdate() {
    if (currentAnimation->HasFinished() && state == FireflyParticleState::FADING)
        Engine::GetInstance().particleManager->DestroyParticle(this);

    return true;
}

bool FireflyParticle::CleanUp() {
    return true;
}

void FireflyParticle::SetPosition(Vector2D pos) {
    position.setX(pos.getX());
    position.setY(pos.getY());
}
