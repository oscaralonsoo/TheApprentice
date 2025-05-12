#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"

struct SDL_Texture;

enum class AbilityZoneStates
{
	WAITING,
	ACTIVATED,
	UNLOCKED
};

class AbilityZone : public Entity
{
public:

	AbilityZone();
	virtual ~AbilityZone();

	bool Awake();

	bool Start();

	bool PreUpdate();

	bool Update(float dt);

	bool PostUpdate() override;

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		
		this->parameters = parameters;

		position.x = parameters.attribute("x").as_int();
		position.y = parameters.attribute("y").as_int();
		type = parameters.attribute("type").as_string();

		texW = parameters.attribute("w").as_int();
		texH = parameters.attribute("h").as_int();
	}
	void VignetteChange(float dt);

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	virtual void OnCollision(PhysBody* physA, PhysBody* physB);

	virtual void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetController(SDL_GameController* controller);

	bool playerInside = false;
	bool playerInsideJump = false;
	bool playerInsideDoubleJump = false;
	bool playerInsideDash = false;

private:
	PhysBody* pbody;
	SDL_Texture* texture;
	std::string type;
	int texW;
	int texH;
	Animation idleAnim;
	Animation* currentAnimation = nullptr;
	pugi::xml_node parameters;
	AbilityZoneStates state = AbilityZoneStates::WAITING;
	SDL_Texture* abilitySprite = nullptr;
	int abilitySpriteW = 0;
	int abilitySpriteH = 0;
	bool markedForDeletion = false;

	int minVignetteSize = 300;
	int maxVignetteSize = 900;
	float vibrateAmplitude = 400.0f;
	float vibrateSpeed = 30.0f;
	int newVignetteSize = 0;
	SDL_Color normalVignetteColor = { 0, 0, 0, 255 };
	bool xHeld = false;
	SDL_GameController* controller = nullptr;
	bool waitingForEatAnimation = false;

	SDL_Texture* jumpSprite = nullptr;
	SDL_Texture* doubleJumpSprite = nullptr;
	SDL_Texture* dashSprite = nullptr;
	SDL_Texture* hookSprite = nullptr;
};
