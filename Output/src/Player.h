#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Box2D/Box2D.h"
#include "PlayerAnimation.h"  // Añadimos la clase de animaciones
#include "Timer.h"

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
	Vector2D GetPosition() const;
	int GetMovementDirection() const { return movementDirection; }
	void EnableDoubleJump(bool enable) { canDoubleJump = enable; }
	void EnableJump(bool enable) { canJump = enable; }

private:
	// Manejo de input
	void HandleInput();
	void HandleJump();
	void HandleDash();

	// Parámetros del jugador
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;

	// Física del jugador
	PhysBody* pbody;
	float jumpForce = 10.5f;
	bool isJumping = false;
	int movementDirection = 0;

	float jumpTime = 0.0f;
	float maxJumpTime = 0.3f;
	bool canJump = true;

	bool canDoubleJump = true;
	bool hasDoubleJumped = false;

	int lastDashDirection = 0;
	Timer dashDuration;
	Timer dashCooldown;
	bool canDash = true;
	b2Vec2 dashImpulse = b2Vec2(10.0f, 0);

	// Animaciones del jugador
	PlayerAnimation animation;
	std::string state = "idle";

	// Parameters config.xml
	pugi::xml_node parameters;

};


