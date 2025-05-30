#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Box2D/Box2D.h"
#include "PlayerAnimation.h"
#include "Timer.h"
#include "PlayerMechanics.h"

struct SDL_Texture;

class PlayerAnimation;

class Player : public Entity
{
public:

	Player();
	virtual ~Player();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool PostUpdate();
	bool CleanUp();

	// Manejo de colisiones
	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}
	void SetPosition(Vector2D pos);
	Vector2D GetPosition() const;

	// Esto lo usa Mechanics
	void SetState(const std::string& newState) { state = newState; }
	const std::string& GetState() const { return state; }

	int GetMovementDirection() const;

	PlayerMechanics* GetMechanics() { return &mechanics; }
	PlayerAnimation* GetAnimation() { return animation; }

	int GetTextureWidth() const;

	int targetScene = 0;
	PhysBody* pbody;
	PhysBody* enemySensor = nullptr;

private:

	SDL_Texture* texture = nullptr;
	int texW, texH;
	std::string state = "idle";

	pugi::xml_node parameters;

	// Nueva clase para manejar todas las mecánicas
	PlayerMechanics mechanics;

	bool initialized = false;

	PlayerAnimation* animation = nullptr;
};
