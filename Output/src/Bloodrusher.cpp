#include "Bloodrusher.h"
#include "Engine.h"

Bloodrusher::Bloodrusher() : Enemy(EntityType::BLOODRUSHER) {
	Awake();
	Start();
}

Bloodrusher::~Bloodrusher() {
}

bool Bloodrusher::Awake() {
    return Enemy::Awake();
}

bool Bloodrusher::Start() {
    return Enemy::Start();
}

bool Bloodrusher::Update(float dt) {
    //SetPosition(position);

    return Enemy::Update(dt);
}


bool Bloodrusher::CleanUp() {
    return Enemy::CleanUp();
}
