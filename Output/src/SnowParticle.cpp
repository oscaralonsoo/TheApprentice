#include "SnowParticle.h"
#include "Engine.h"
#include "Scene.h"
#include "ParticleManager.h"
#include "Textures.h"
#include <cmath>

SnowParticle::SnowParticle()
    : Entity(EntityType::PARTICLE)
{
    velocity = { 0.0f, 0.0f };
}

SnowParticle::~SnowParticle() {
}

bool SnowParticle::Awake() {
    return true;
}

bool SnowParticle::Start() {
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    pugi::xml_node snowNode = loadFile.child("config").child("scene").child("animations").child("particles").child("snow");

    texture = Engine::GetInstance().textures->Load(snowNode.attribute("texture").as_string());
    idleAnim.LoadAnimations(snowNode.child("idle"));

    scale = 0.2f + static_cast<float>(rand()) / RAND_MAX * (0.7f - 0.2f);

    texW = snowNode.attribute("w").as_int() * scale;
    texH = snowNode.attribute("h").as_int() * scale;

    velocity.y = 0.8f + static_cast<float>(rand()) / RAND_MAX * (2.0f - 0.8f);

    oscillationAmplitude = 8.0f + static_cast<float>(rand()) / RAND_MAX * (8.0f - 8.0f);
    oscillationFrequency = 2.0f * M_PI * (0.1f + static_cast<float>(rand()) / RAND_MAX * (0.4f - 0.1f));

    phaseOffset = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;

    currentAnimation = &idleAnim;

    return true;
}

bool SnowParticle::Update(float dt) {
    time += dt / 1000.0f;
    float offsetX = oscillationAmplitude * sinf(oscillationFrequency * time + phaseOffset);

    position.setX(baseX + offsetX);
    position.setY(position.getY() + velocity.y);

    Engine::GetInstance().render->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_NONE, scale);

    currentAnimation->Update();

    return true;
}

bool SnowParticle::PostUpdate() {
    Vector2D camPos = {
        Engine::GetInstance().render->camera.x * -1.0f,
        Engine::GetInstance().render->camera.y * -1.0f
    };

    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &windowWidth, &windowHeight);

    float bottomLimit = camPos.y + windowHeight;

    if (position.getY() > bottomLimit) {
        Engine::GetInstance().particleManager->DestroyParticle(this);
    }

    return true;
}

bool SnowParticle::CleanUp() {
    return true;
}

void SnowParticle::SetPosition(Vector2D pos) {
    position.setX(pos.getX());
    position.setY(pos.getY());
    baseX = pos.getX();
}
