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
    return true;
}

bool Geyser::Update(float dt)
{
    switch (state) {
    case GeyserState::DISABLED:
        if (currentAnimation != &disabledAnim) currentAnimation = &disabledAnim;
        if (geyserTimer.ReadMSec() >= geyserCooldown) state = GeyserState::ENABLED;
        break;
    case GeyserState::ENABLED:
        if (currentAnimation != &enabledAnim) currentAnimation = &enabledAnim;

        if (currentAnimation->HasFinished())
        {
            currentAnimation->Reset();
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
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        if (state == GeyserState::ENABLED)
        {
            Player* player = Engine::GetInstance().scene.get()->GetPlayer();
            player->pbody->body->SetLinearVelocity(b2Vec2_zero);
            player->pbody->body->ApplyLinearImpulse(b2Vec2(0.0f, -23.0f), b2Vec2(player->GetPosition().x, player->GetPosition().y), true);
        }
        break;
    }
}
void Geyser::OnCollisionEnd(PhysBody* physA, PhysBody* physB){
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        break;
    }
}


void Geyser::RenderTexture() {
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - (texW / 2));
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - (texH / 2));

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + width / 2 - texW / 2, (int)position.getY() + height - texH, &currentAnimation->GetCurrentFrame());
    currentAnimation->Update();
}
