#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Physics.h"
#include "Module.h"

Enemy::Enemy(EntityType type) : Entity(type)
{
}

Enemy::~Enemy() {
	delete pathfinding;
}

bool Enemy::Awake() {
	return true;
}

bool Enemy::Start() {
	//Assign collider type
	pbody->ctype = ColliderType::ENEMY;

	pbody->listener = this;

	// Set the gravity of the body
	if (!gravity) pbody->body->SetGravityScale(0);

	// Initialize pathfinding
	pathfinding = new Pathfinding();
	ResetPath();

	b2Fixture* fixture = pbody->body->GetFixtureList();
	if (fixture) {
		b2Filter filter;
		filter.categoryBits = CATEGORY_ENEMY;
		filter.maskBits = CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_ATTACK | CATEGORY_PLAYER_DAMAGE;
		fixture->SetFilterData(filter);
	}

	return true;
}

bool Enemy::Update(float dt)
{
	// Propagate the pathfinding algorithm using A* with the selected heuristic
	ResetPath();	

	steps = 0;
	while (pathfinding->pathTiles.empty() && steps < maxSteps) {
		pathfinding->PropagateAStar(SQUARED);
		steps++;
	}

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	SDL_RendererFlip flip = SDL_FLIP_NONE;
	if (direction > 0) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
	if (upsiteDown)    flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);

	Engine::GetInstance().render.get()->DrawTexture(texture,
		(int)position.getX(),
		(int)position.getY() - 15,
		&currentAnimation->GetCurrentFrame(),
		1.0f,
		0.0,
		INT_MAX,
		INT_MAX,
		flip,
		scale
	);
	currentAnimation->Update();

	//Show|Hide Path
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) {
		showPath = !showPath;
	}
	if (showPath) {
		pathfinding->DrawPath();
	}

	return true;
}

bool Enemy::PostUpdate()
{
	return true;
}

bool Enemy::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void Enemy::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Enemy::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Enemy::ResetPath() {
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos);
}

void Enemy::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::ATTACK:
		b2Fixture* fixture = pbody->body->GetFixtureList();
		b2Filter filter = fixture->GetFilterData();
		filter.maskBits &= ~CATEGORY_PLAYER_DAMAGE;
		fixture->SetFilterData(filter);
		break;
	}
}

void Enemy::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::ATTACK:

		break;
	}
}