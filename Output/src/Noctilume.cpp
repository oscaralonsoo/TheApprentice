#include "Engine.h"
#include "Physics.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Textures.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Log.h"
#include <cmath>
#include "Noctilume.h"

Noctilume::Noctilume() : Enemy(EntityType::NOCTILUME)
{
}
Noctilume::~Noctilume()
{
}

bool Noctilume::Awake()
{
	return false;
}

bool Noctilume::Start()
{
    pugi::xml_document configDoc;
    if (!configDoc.load_file("config.xml")) {
        return false;
    }

    std::string typeName = "Noctilume";
    for (pugi::xml_node node : configDoc.child("config").child("scene").child("animations").child("enemies").children("enemy")) {
        if (node.attribute("type").as_string() == typeName) {
            texture = Engine::GetInstance().textures->Load(node.attribute("texture").as_string());
            idleAnim.LoadAnimations(node.child("idle"));
            currentAnimation = &idleAnim;
            break;
        }
    }
    pbody = Engine::GetInstance().physics.get()->CreateCircleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

    pbody->ctype = ColliderType::ENEMY;

    pbody->listener = this;

    if (!gravity) pbody->body->SetGravityScale(0);

    pathfinding = new Pathfinding();
    ResetPath();

    return true;
}

bool Noctilume::Update(float dt)
{
    if (DistanceToPlayer() < 300.0f && DistanceToPlayer() > 100.0f) {
        currentState = NoctilumeState::FLYING;
    }
    switch (currentState)
    {
    case NoctilumeState::IDLE:
        IdleFlight(dt);
        break;
    case NoctilumeState::FLYING:
        Flying(dt);
        break;
    case NoctilumeState::DIVE:
        Dive(dt);
        break;
    case NoctilumeState::DEAD:
        break;
    }


    return Enemy::Update(dt);
}


void Noctilume::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLAYER:
        break;
    case ColliderType::ATTACK:

        break;
    }
}

bool Noctilume::CleanUp()
{
	return false;
}
void Noctilume::IdleFlight(float dt)
{
    // Se tiene que mover de derecha a izquierda lentamente por la parte de arriba sin colisionar con nada subiendo, bajando y rectificando
}
void Noctilume::Flying(float dt)
{
    // Se mueve de derecha a izquierda siguiendo al jugador desde la altura preparando el ataque de dive ( caida en picado en forma de U )
}
void Noctilume::Dive(float dt)
{
    // Caida en picado hacia la posicion del player al iniciar la caida, caida en forma de U y vuelve a subir al cielo Flying o IdleFlying
}



float Noctilume::DistanceToPlayer()
{
    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();

    float deltaX = playerPos.x - position.x;
    float deltaY = playerPos.y - position.y;

    return sqrt(deltaX * deltaX + deltaY * deltaY);
}
