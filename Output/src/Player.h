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

	//Unlock Abilities
	void EnableDoubleJump(bool enable) { doubleJumpUnlocked = enable; }
	void EnableJump(bool enable) { jumpUnlocked = enable; }
	void EnableDash(bool enable) { dashUnlocked = enable; }


private:
	// Manejo de input
	void HandleInput();
	void HandleJump();
	void HandleDash();
	void HandleFall(); 
	void CheckFallImpact();
	void HandleWallSlide();
	void CancelDash();
	void CreateAttackSensor();
	void DestroyAttackSensor();

	// Parámetros del jugador
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;

	// Física del jugador
	PhysBody* pbody;
	bool isOnGround = false;
	
	//Know Direction
	int movementDirection = 1;
	
	//Jump
	float jumpForce = 10.5f;
	bool isJumping = false;
	float jumpTime = 0.0f;
	float maxJumpTime = 0.3f;
	bool jumpUnlocked = false;

	//DoubleJump
	bool doubleJumpUnlocked = false;
	bool hasDoubleJumped = false;

	//Dash
	bool isDashing = false;
	bool canDash = true;
	float dashSpeed = 15.0f;         
	Vector2D dashStartPosition;
	float maxDashDistance = 100.0f; 
	Timer dashCooldown;
	float dashMaxCoolDown = 1.0f;
	bool dashUnlocked = false;

	//Fall Stun
	bool isStunned = false;
	float stunDuration = 1.0f;
	Timer stunTimer;
	float fallStartY = 0.0f;
	float fallDistanceThreshold = 300.0f; 
	float fallEndY;
	float fallDistance;

	//Bajar la camara al lado de bajadas
	bool wasInDownCameraZone = false;

	//Wall Slide
	bool isWallSliding = false;

	//Attack
	PhysBody* attackSensor = nullptr;
	Timer attackTimer;
	float attackDuration = 200.0f;
	int size = 50;
	int playerAttackX;
	int playerAttackY;


	// Animaciones del jugador
	PlayerAnimation animation;
	std::string state = "idle";

	// Parameters config.xml
	pugi::xml_node parameters;

};


