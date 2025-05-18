#include "NullwardenCrystal.h"
#include "Engine.h"
#include "Textures.h"
#include "Physics.h"
#include "Scene.h"
#include "Nullwarden.h"
#include "EntityManager.h"

NullwardenCrystal::NullwardenCrystal(float x, float y, float speed, b2Vec2 dir, Nullwarden* owner)
    : Entity(EntityType::CRYSTAL), direction(dir), nullwarden(owner)
{
    position.setX(x);
    position.setY(y);
    direction = dir;

    texture = Engine::GetInstance().textures->Load("textures/nullwarden_crystal.png");

    // Cargar animaciones (asumiendo que están definidas en un XML)
    pugi::xml_document animDoc;
    animDoc.load_file("config.xml");
    auto crystalNode = animDoc.child("config").child("scene").child("animations").child("NullwardenCrystal");

    pristineAnim.LoadAnimations(crystalNode.child("pristine"));
    crackedAnim.LoadAnimations(crystalNode.child("cracked"));
    shatteredAnim.LoadAnimations(crystalNode.child("shattered"));
    breakAnim.LoadAnimations(crystalNode.child("broken"));

    currentAnimation = &pristineAnim;

    width = currentAnimation->GetCurrentFrame().w;
    height = currentAnimation->GetCurrentFrame().h;

    pbody = Engine::GetInstance().physics->CreateRectangle(x + width / 2, y + height / 2, width, height, bodyType::STATIC);
    pbody->ctype = ColliderType::ENEMY; // O usar uno nuevo como SHIELD
    pbody->listener = this;
}

NullwardenCrystal::~NullwardenCrystal() {}

bool NullwardenCrystal::Update(float dt) {
    currentAnimation->Update();
    Engine::GetInstance().render->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
    return true;
}

bool NullwardenCrystal::CleanUp() {
    return true;
}

void NullwardenCrystal::OnCollision(PhysBody* physA, PhysBody* physB) {
    if (physB->ctype == ColliderType::ATTACK && hits < 4) {
        hits++;
        switch (hits) {
        case 0:
            currentAnimation = &pristineAnim;
            break;
        case 1:
            currentAnimation = &crackedAnim;
            break;
        case 2:
            break;
            currentAnimation = &shatteredAnim;
        case 3:
            currentAnimation = &breakAnim;
            nullwarden->crystalBroken = true;
            if (currentAnimation->HasFinished()) {
                Engine::GetInstance().entityManager->DestroyEntity(this);
            }
            break;
        }
    }
}
