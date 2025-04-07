#include "MirebornFragment.h"
#include "Engine.h"

MirebornFragment::MirebornFragment() : Enemy(EntityType::MIREBORN_FRAGMENT) {
}

MirebornFragment::~MirebornFragment() {
}

bool MirebornFragment::Awake() {
    return Enemy::Awake();
}

bool MirebornFragment::Start() {
    // Cargar texturas y animaciones específicas para el fragmento
    return Enemy::Start();
}

bool MirebornFragment::Update(float dt) {
    // Lógica de actualización específica para el fragmento
    return Enemy::Update(dt);
}

bool MirebornFragment::CleanUp() {
    return Enemy::CleanUp();
}

void MirebornFragment::OnCollision(PhysBody* physA, PhysBody* physB) {
    // Lógica de colisión específica para el fragmento
    if (physB->ctype == ColliderType::ATTACK) {
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }
}