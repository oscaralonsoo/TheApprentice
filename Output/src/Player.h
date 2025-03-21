#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Box2D/Box2D.h"
#include "PlayerAnimation.h"  // Añadimos la clase de animaciones

struct SDL_Texture;

class Player : public Entity
{
public:

	Player();
	virtual ~Player();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool CleanUp();

	// Manejo de colisiones
	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	int GetMovementDirection() const { return movementDirection; }
	Vector2D GetPosition() const;
	void SetPosition(Vector2D pos);

private:
	// Manejo de input
	void HandleInput();
	void HandleJump();

	// Parámetros del jugador
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;

	// Física del jugador
	PhysBody* pbody;
	float jumpForce = 10.5f;
	bool isJumping = false;
	int movementDirection = 0;

	// Animaciones del jugador
	PlayerAnimation animation;
	std::string state = "idle";

	// Parameters config.xml
	pugi::xml_node parameters;

};
