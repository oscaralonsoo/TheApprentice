#include "Engine.h"
#include "Window.h"
#include "Render.h"
#include "Log.h"
#include "Player.h"
#include "Map.h"
#include <SDL2/SDL_image.h>
#include "SDL2/SDL_ttf.h"

#define VSYNC true

Render::Render() : Module()
{
	name = "render";
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
Render::~Render()
{}

// Called before render is available
bool Render::Awake()
{
	LOG("Create SDL rendering context");
	bool ret = true;

	Uint32 flags = SDL_RENDERER_ACCELERATED;

	//L05 TODO 5 - Load the configuration of the Render module
	if (configParameters.child("vsync").attribute("value").as_bool() == true) {
		flags |= SDL_RENDERER_PRESENTVSYNC;
		LOG("Using vsync");
	}
	int scale = Engine::GetInstance().window.get()->GetScale();

	SDL_Window* window = Engine::GetInstance().window.get()->window;
	renderer = SDL_CreateRenderer(window, -1, flags);

	if(renderer == NULL)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		camera.w = Engine::GetInstance().window.get()->width * scale;
		camera.h = Engine::GetInstance().window.get()->height * scale;
		camera.x = 0;
		camera.y = 0;
	}
	//Initialize the TTF library
	TTF_Init();
	//Load a font into memory
	font = TTF_OpenFont("Assets/Fonts/ChangesModern.ttf", 72);
	return ret;
}

// Called before the first frame
bool Render::Start()
{
	LOG("render start");
	// back background
	SDL_RenderGetViewport(renderer, &viewport);
	return true;
}

// Called each loop iteration
bool Render::PreUpdate()
{
	SDL_RenderClear(renderer);
	return true;
}

bool Render::Update(float dt)
{
	return true;
}

bool Render::PostUpdate()
{
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.b, background.a);
	SDL_RenderPresent(renderer);

	return true;
}

// Called before quitting
bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_RenderSetViewport(renderer, &rect);
}

void Render::ResetViewPort()
{
	SDL_RenderSetViewport(renderer, &viewport);
}

// Blit to screen
bool Render::DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window.get()->GetScale();

	SDL_Rect rect;
	rect.x = (int)(camera.x * speed) + x * scale;
	rect.y = (int)(camera.y * speed) + y * scale;

	if(section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	}

	rect.w *= scale;
	rect.h *= scale;

	SDL_Point* p = NULL;
	SDL_Point pivot;

	if(pivotX != INT_MAX && pivotY != INT_MAX)
	{
		pivot.x = pivotX;
		pivot.y = pivotY;
		p = &pivot;
	}

	if(SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, SDL_FLIP_NONE) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window.get()->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_Rect rec(rect);
	if(use_camera)
	{
		rec.x = (int)(camera.x + rect.x * scale);
		rec.y = (int)(camera.y + rect.y * scale);
		rec.w *= scale;
		rec.h *= scale;
	}

	int result = (filled) ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window.get()->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;

	if(use_camera)
		result = SDL_RenderDrawLine(renderer, camera.x + x1 * scale, camera.y + y1 * scale, camera.x + x2 * scale, camera.y + y2 * scale);
	else
		result = SDL_RenderDrawLine(renderer, x1 * scale, y1 * scale, x2 * scale, y2 * scale);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window.get()->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_Point points[360];

	float factor = (float)M_PI / 180.0f;

	for(int i = 0; i < 360; ++i)
	{
		points[i].x = (int)(x * scale + camera.x) + (int)(radius * cos(i * factor));
		points[i].y = (int)(y * scale + camera.y) + (int)(radius * sin(i * factor));
	}

	result = SDL_RenderDrawPoints(renderer, points, 360);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

void Render::UpdateCamera(const Vector2D& targetPosition, int movementDirection, float smoothing)
{
	if (cameraLocked)
		return;

	cameraYOffset += (targetCameraYOffset - cameraYOffset) * yOffsetSmoothing;
	
	int offsetX = -110 * movementDirection;

	mapWidthPx = Engine::GetInstance().map->GetMapWidth();
	mapHeightPx = Engine::GetInstance().map->GetMapHeight();
	
	targetX = static_cast<int>(targetPosition.x);
	targetY = static_cast<int>(targetPosition.y);

	// ----------- Cámara look-ahead horizontal con retardo -----------

	if (movementDirection != 0) {
		if (movementDirection == lastMoveDir) {
			lookAheadCounter++;
			if (lookAheadCounter >= lookAheadDelayFrames) {
				cameraLookAheadTarget = lookAheadDistance * movementDirection;
			}
		}
		else {
			lookAheadCounter = 0;
			cameraLookAheadTarget = 0;
		}
		lastMoveDir = movementDirection;
	}
	else {
		cameraLookAheadTarget = 0;
		lookAheadCounter = 0;
		lastMoveDir = 0;
	}

	// Suavizado del desplazamiento hacia el objetivo
	cameraLookAheadOffset = EaseInOut(cameraLookAheadOffset, cameraLookAheadTarget, lookAheadSmoothing);
	
	int targetCamX = -targetX + camera.w / 2 - static_cast<int>(cameraLookAheadOffset) + cameraImpulseX;
	camera.x += static_cast<int>((targetCamX - camera.x) * smoothing);

	cameraImpulseX = static_cast<int>(cameraImpulseX * (1.0f - cameraImpulseSmoothing));

	if (!isYOffsetLocked)
	{
		cameraCenterY = -camera.y + camera.h / 2 - static_cast<int>(cameraYOffset);

		if (targetY < cameraCenterY - followMargin || targetY > cameraCenterY + followMargin)
		{
			camera.y += static_cast<int>((-targetY + camera.h / 2 + cameraVerticalViewOffset - camera.y) * smoothing);
		}
	}
	else
	{
		int lockedY = -targetY + camera.h / 2 + static_cast<int>(cameraYOffset) + cameraVerticalViewOffset;
		camera.y += static_cast<int>((lockedY - camera.y) * smoothing);
	}


	// Aplicar shake
	if (isShaking) {
		if (shakeTimer.ReadSec() >= shakeDurationSec) {
			isShaking = false;
			shakeOffsetX = 0;
			shakeOffsetY = 0;
		}
		else {
			shakeOffsetX = (rand() % (shakeIntensity * 2 + 1)) - shakeIntensity;
			shakeOffsetY = (rand() % (shakeIntensity * 2 + 1)) - shakeIntensity;
		}
	}
	else {
		shakeOffsetX = 0;
		shakeOffsetY = 0;
	}

	camera.x += shakeOffsetX;
	camera.y += shakeOffsetY;

	if (camera.x > 0) camera.x = 0;  
	if (camera.y > 0) camera.y = 0;  
	if (camera.x < -(mapWidthPx - camera.w)) camera.x = -(mapWidthPx - camera.w);
	if (camera.y < -(mapHeightPx - camera.h)) camera.y = -(mapHeightPx - camera.h);
}
bool Render::DrawText(const char* text, int posx, int posy, int w, int h, SDL_Color color) const
{
	SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	int texW = 0;
	int texH = 0;
	SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
	SDL_Rect dstrect = { posx, posy, w, h };
	SDL_RenderCopy(renderer, texture, NULL, &dstrect);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	return true;
}
SDL_Texture* Render::LoadTexture(const char* path)
{
	SDL_Texture* texture = IMG_LoadTexture(renderer, path);
	if (!texture)
	{
		LOG("Failed to load texture: %s", SDL_GetError());
	}
	return texture;
}

void Render::DashCameraImpulse(int direction, int intensity)
{
	cameraImpulseX = -direction * intensity;
}

void Render::StartCameraShake(int durationSec, int intensity)
{
	isShaking = true;
	shakeDurationSec = durationSec;
	shakeIntensity = intensity;
	shakeTimer.Start();
}

void Render::ToggleCameraLock()
{
	cameraLocked = !cameraLocked;

	if (cameraLocked)
	{
		// Centrar la cámara en el centro del mapa (o donde prefieras)
		mapWidthPx = Engine::GetInstance().map->GetMapWidth();
		mapHeightPx = Engine::GetInstance().map->GetMapHeight();

		camera.x = -(mapWidthPx / 2 - camera.w / 2);
		camera.y = -(mapHeightPx / 2 - camera.h / 2);
	}
}

void Render::ToggleVerticalOffsetLock()
{
	isYOffsetLocked = !isYOffsetLocked;

	if (isYOffsetLocked)
	{
		targetCameraYOffset = static_cast<float>(defaultYOffset);
	}
	else
	{
		targetCameraYOffset = 0.0f;
	}
}

float Render::EaseInOut(float current, float target, float t)
{
	float delta = target - current;
	float easedT = t * t * (3 - 2 * t); // curva tipo easeInOutCubic
	return current + delta * easedT;
}


