#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"

struct SDL_Texture;

class NPC : public Entity
{
public:

	NPC(EntityType type);
	virtual ~NPC();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool PostUpdate();

	bool CleanUp();

	virtual void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;

		position.x = parameters.attribute("x").as_int();
		position.y = parameters.attribute("y").as_int();
		width = parameters.attribute("w").as_int();
		height = parameters.attribute("h").as_int();
		type = parameters.attribute("type").as_string();
		dialogueId = parameters.attribute("dialogueId").as_int();
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	virtual void OnCollision(PhysBody* physA, PhysBody* physB);

	virtual void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	std::string GetNPCType() const { return type; }

private:
	PhysBody* pbody;

	pugi::xml_node parameters;
	bool gravity;
	std::string type;
	int width, height;

	SDL_Texture* texture;
	Animation* currentAnimation = nullptr;
	Animation idleAnim;

	int dialogueId;
};
