#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Map.h"
#include "Physics.h"
#include "Module.h"
#include "HiddenZone.h"

HiddenZone::HiddenZone() : Entity(EntityType::HIDDEN_ZONE)
{
}

HiddenZone::~HiddenZone() {
}

bool HiddenZone::Awake() {
	return true;
}

bool HiddenZone::Start() {

	//Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + width/2, (int)position.getY() + height/2, width, height, bodyType::STATIC);

	//Assign collider type
	pbody->ctype = ColliderType::HIDDEN_ZONE;

	pbody->listener = this;

	return true;
}

bool HiddenZone::Update(float dt)
{
	if (fadingIn)
	{
		alpha += (int)(fadeSpeed * dt);
		if (alpha >= 255)
		{
			alpha = 255;
			fadingIn = false;
		}
	}
	else if (fadingOut)
	{
		alpha -= (int)(fadeSpeed * dt);
		if (alpha <= 0)
		{
			alpha = 0;
			fadingOut = false;
		}
	}

	SDL_Rect mainRect = { (int)position.getX(), (int)position.getY(), width, height };

	// Dibuja el rectángulo principal
	Engine::GetInstance().render->DrawRectangle(mainRect, 0, 0, 0, alpha, true, true);

	// Dibujar borde degradado si alpha > 0
	if (alpha > 0)
	{
		const int borderSize = 64;
		const int steps = 64;
		const int stepSize = borderSize / steps;

		for (int i = 1; i <= steps; ++i)
		{
			// Proporcional al alpha actual (se desvanece junto al rectángulo)
			float factor = 1.0f - (float)i / (steps + 1);
			int edgeAlpha = static_cast<int>(alpha * factor);

			SDL_Rect edgeRect = {
				mainRect.x - i * stepSize,
				mainRect.y - i * stepSize,
				mainRect.w + 2 * i * stepSize,
				mainRect.h + 2 * i * stepSize
			};

			Engine::GetInstance().render->DrawRectangle(edgeRect, 0, 0, 0, edgeAlpha, true, true);
		}
	}

	return true;
}



bool HiddenZone::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void HiddenZone::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {
	case ColliderType::PLAYER:
		fadingIn = false;
		fadingOut = true;
		break;
	}
}

void HiddenZone::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype) {
	case ColliderType::PLAYER:
		fadingOut = false;
		fadingIn = true;
		break;
	}
}

void HiddenZone::SetWidth(int width)
{
	this->width = width;
}

void HiddenZone::SetHeight(int height)
{
	this->height = height;
}
