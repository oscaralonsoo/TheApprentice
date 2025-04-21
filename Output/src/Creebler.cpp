#include "Creebler.h"
#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"


Creebler::Creebler() : Enemy(EntityType::CREEBLER) {
}

Creebler::~Creebler() {
}

bool Creebler::Awake() {
    return Enemy::Awake();
}

bool Creebler::Start() {
    //Add a physics to an item - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texW / 2, (int)position.getY() + texH / 2, texW / 1.3, texH / 1.9, bodyType::DYNAMIC);

    //Assign collider type
    pbody->ctype = ColliderType::ENEMY;

    pbody->listener = this;

    // Set the gravity of the body
    if (!gravity) pbody->body->SetGravityScale(0);

    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");

    for (pugi::xml_node enemyNode = loadFile.child("config").child("scene").child("animations").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
    {
        if (std::string(enemyNode.attribute("type").as_string()) == type)
        {
            texture = Engine::GetInstance().textures.get()->Load(enemyNode.attribute("texture").as_string());
            walkAnim.LoadAnimations(enemyNode.child("walk"));
            deadAnim.LoadAnimations(enemyNode.child("dead"));
        }
    }

    currentAnimation = &walkAnim;

    return true;
}

bool Creebler::Update(float dt) {
    switch (currentState)
    {
    case CreeblerState::WALKING:
        if (currentAnimation != &walkAnim) currentAnimation = &walkAnim;
        Walk();
        break;
    case CreeblerState::DEAD:
        if (currentAnimation != &deadAnim) currentAnimation = &deadAnim;

        pbody->body->SetLinearVelocity(b2Vec2_zero);
        pbody->body->SetAngularVelocity(0);
        pbody->body->GetFixtureList()->SetSensor(true);
        pbody->body->SetGravityScale(0);
        break;
    }

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY()-15, &currentAnimation->GetCurrentFrame(),
        1.0f,
        0.0,
        INT_MAX,
        INT_MAX,
        (direction < 0) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
    currentAnimation->Update();

    return true;
}


bool Creebler::PostUpdate() {
    
    if (currentState == CreeblerState::DEAD && currentAnimation->HasFinished()) {
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }

    return true;
}

bool Creebler::CleanUp() {
    return Enemy::CleanUp();
}

void Creebler::Walk() {
    pbody->body->SetLinearVelocity(b2Vec2(direction * speed, pbody->body->GetLinearVelocity().y));

    Vector2D posMap = Engine::GetInstance().map.get()->WorldToMap(position.getX() + texW / 2, position.getY() + texH / 2);

    // Tile enfrente
    int frontX = posMap.x + direction;
    int frontY = posMap.y + 1;

    MapLayer* layer = Engine::GetInstance().map.get()->GetNavigationLayer();

    if (layer->Get(frontX, posMap.y) || !layer->Get(frontX, frontY))
        direction *= -1;
}

void Creebler::OnCollision(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::ATTACK:
        currentState = CreeblerState::DEAD;
        break;
    }
}