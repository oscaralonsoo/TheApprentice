#include "Geyser.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Player.h"

Geyser::Geyser() : Entity(EntityType::GEYSER), state(GeyserState::DISABLED)
{
    name = "Geyser";
}

Geyser::~Geyser()
{
}

bool Geyser::Awake()
{
	return false;
}

bool Geyser::Start()
{
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    pugi::xml_node geyserNode = loadFile.child("config").child("scene").child("animations").child("props").child("geyser");

    texture = Engine::GetInstance().textures->Load(geyserNode.attribute("texture").as_string());
    disabledAnim.LoadAnimations(geyserNode.child("disabled"));
    enabledAnim.LoadAnimations(geyserNode.child("enabled"));

    texW = geyserNode.attribute("w").as_int();
    texH = geyserNode.attribute("h").as_int();

    pbody = Engine::GetInstance().physics->CreateRectangleSensor(
        (int)position.getX() + width / 2,
        (int)position.getY() + height / 2,
        width, height,
        STATIC,
        CATEGORY_GEYSER,
        CATEGORY_PLAYER || CATEGORY_PLATFORM
    );

    pbody->ctype = ColliderType::GEYSER;
    pbody->listener = this;
    pbody->body->SetGravityScale(0);

    currentAnimation = &disabledAnim;

    geyserTimer.Start();

    soundInteractId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Props/geiser_sound.ogg", 1.0f);

    return true;
}

bool Geyser::Update(float dt)
{
    switch (state) {
    case GeyserState::DISABLED:
        if (currentAnimation != &disabledAnim) currentAnimation = &disabledAnim;
        if (geyserTimer.ReadMSec() >= geyserCooldown) {
            state = GeyserState::ENABLED;
            hasPushed = false;
        }
        break;

    case GeyserState::ENABLED:

        interactSoundTimer -= dt;
        if (interactSoundTimer <= 0.0f) {
            Engine::GetInstance().audio->PlayFx(soundInteractId, 1.0f, 0);
            interactSoundTimer = interactSoundInterval;
        }

        if (currentAnimation != &enabledAnim) {
            currentAnimation = &enabledAnim;

            currentAnimation->Reset();
        }

        if (playerInside && !hasPushed)
        {

            Player* player = Engine::GetInstance().scene.get()->GetPlayer();
            player->pbody->body->SetLinearVelocity(b2Vec2_zero);
            player->pbody->body->ApplyLinearImpulse(b2Vec2(0.0f, -23.0f), player->pbody->body->GetWorldCenter(), true);
            hasPushed = true;
        }

        if (currentAnimation->HasFinished())
        {
            
            state = GeyserState::DISABLED;
            geyserTimer.Start();
        }
        break;
    }

    RenderTexture();

    return true;
}

bool Geyser::PostUpdate()
{
	return true;
}

bool Geyser::CleanUp()
{
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void Geyser::OnCollision(PhysBody* physA, PhysBody* physB) {
    if (physB->ctype == ColliderType::PLAYER) {
        playerInside = true;
    }
}

void Geyser::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    if (physB->ctype == ColliderType::PLAYER) {
        playerInside = false;
        hasPushed = false;
    }
}


void Geyser::RenderTexture() {
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - (texW / 2));
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH);

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY() + height, &currentAnimation->GetCurrentFrame());
    currentAnimation->Update();
}
