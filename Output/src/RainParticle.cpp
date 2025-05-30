#include "RainParticle.h"
#include "Engine.h"
#include "Scene.h"
#include "ParticleManager.h"
#include "Textures.h"
#include <cmath>

RainParticle::RainParticle()
    : Entity(EntityType::PARTICLE)
{
    velocity = { 0.0f, 0.0f };
}

RainParticle::~RainParticle() {
}

bool RainParticle::Awake() {
    return true;
}

bool RainParticle::Start() {
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    pugi::xml_node rainNode = loadFile.child("config").child("scene").child("animations").child("particles").child("rain");


    texture = Engine::GetInstance().textures->Load(rainNode.attribute("texture").as_string());
    idleAnim.LoadAnimations(rainNode.child("idle"));

    scale = 0.3f + static_cast<float>(rand()) / RAND_MAX * (1.0f - 0.3f);

    texW = rainNode.attribute("w").as_int() * scale;
    texH = rainNode.attribute("h").as_int() * scale;

    velocity.y = 7.0f + static_cast<float>(rand()) / RAND_MAX * (10.0f - 7.0f);

    currentAnimation = &idleAnim;

    return true;
}

bool RainParticle::Update(float dt) {
    position.setX(position.getX() + velocity.x);
    position.setY(position.getY() + velocity.y);

    Engine::GetInstance().render->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_NONE, SDL_FLIP_NONE, scale);

    currentAnimation->Update();

    return true;
}

bool RainParticle::PostUpdate() {
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

bool RainParticle::CleanUp() {
    return true;
}

void RainParticle::SetPosition(Vector2D pos) {
    position.setX(pos.getX());
    position.setY(pos.getY());
}
