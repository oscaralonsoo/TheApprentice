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

    return Enemy::Start();
}

bool Noctilume::Update(float dt)
{
    if (pathfinding->HasFoundPlayer()) {
        if (currentState == NoctilumeState::IDLE && DistanceToPlayer() < 10.0f) {
            currentState = NoctilumeState::DIVE;
        }
    }
    switch (currentState)
    {
    case NoctilumeState::IDLE:
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
void Noctilume::Flying(float dt)
{

}

void Noctilume::Dive(float dt)
{
   
}

float Noctilume::DistanceToPlayer()
{
    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();


    float deltaX = playerPos.x - position.x;
    float deltaY = playerPos.y - position.y;

    return sqrt(deltaX * deltaX + deltaY * deltaY);
}

