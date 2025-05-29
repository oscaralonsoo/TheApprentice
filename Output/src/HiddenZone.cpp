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
	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor(
		(int)position.getX() + width / 2,
		(int)position.getY() + height / 2,
		width, height,
		bodyType::STATIC,
		CATEGORY_HIDDEN_ZONE,     // categoría propia
		CATEGORY_PLAYER           // solo colisiona con el jugador
	);


	//Assign collider type
	pbody->ctype = ColliderType::HIDDEN_ZONE;

	pbody->listener = this;

	return true;
}

bool HiddenZone::PostUpdate()
{
	if (fadingOut)
	{
		alpha -= (int)(fadeSpeed);
		if (alpha <= 0)
		{
			alpha = 0;
			fadingOut = false;
			alreadyRevealed = true;
		}
	}

	int x = (int)position.getX();
	int y = (int)position.getY();
	int gradientThickness = 80;

	for (int i = 0; i < gradientThickness; ++i)
	{
		Uint8 gradientAlpha = (Uint8)(((float)(i) / (gradientThickness - 1)) * alpha);
		SDL_Rect borderRect = { x + i, y + i, width - 2 * i, height - 2 * i };
		Engine::GetInstance().render->DrawRectangle(borderRect, 0, 0, 0, gradientAlpha, false, true);
	}

	SDL_Rect mainRect = { x + gradientThickness, y + gradientThickness, width - 2 * gradientThickness, height - 2 * gradientThickness };
	Engine::GetInstance().render->DrawRectangle(mainRect, 0, 0, 0, alpha, true, true);

	return true;
}


bool HiddenZone::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void HiddenZone::OnCollision(PhysBody* physA, PhysBody* physB) {
	if (physB->ctype == ColliderType::PLAYER && !alreadyRevealed)
	{
		fadingOut = true;
	}
}


void HiddenZone::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
}

void HiddenZone::SetWidth(int width)
{
	this->width = width;
}

void HiddenZone::SetHeight(int height)
{
	this->height = height;
}
