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
		cameraZoom = 1.0f;
		targetCameraZoom = 1.0f;
		camera.x = 0;
		camera.y = 0;
	}
	//Initialize the TTF library
	TTF_Init();
	//Load a font into memory
	
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
bool  Render::DrawTexture(SDL_Texture* texture, uint32_t x, uint32_t y, const SDL_Rect* section, float speed, double angle, uint32_t pivotX, uint32_t pivotY, SDL_RendererFlip flip, float scale) const
{
	bool ret = true;
	float windowScale = Engine::GetInstance().window->GetScale() * cameraZoom;

	SDL_Rect rect;
	rect.x = static_cast<int>(camera.x * speed + x * windowScale);
	rect.y = static_cast<int>(camera.y * speed + y * windowScale);


	if(section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	}

	rect.w = static_cast<int>(rect.w * windowScale * scale);
	rect.h = static_cast<int>(rect.h * windowScale * scale);


	SDL_Point* p = NULL;
	SDL_Point pivot;

	if(pivotX != INT_MAX && pivotY != INT_MAX)
	{
		pivot.x = pivotX;
		pivot.y = pivotY;
		p = &pivot;
	}

	if (SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, flip) != 0)
	{
		//LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	float scale = Engine::GetInstance().window->GetScale() * cameraZoom;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_Rect rec(rect);
	if(use_camera)
	{
		rec.x = static_cast<int>(camera.x + rect.x * scale);
		rec.y = static_cast<int>(camera.y + rect.y * scale);
		rec.w = static_cast<int>(rect.w * scale);
		rec.h = static_cast<int>(rect.h * scale);
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
	float scale = Engine::GetInstance().window->GetScale() * cameraZoom;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;

	if (use_camera)
		result = SDL_RenderDrawLine(renderer,
			static_cast<int>(camera.x + x1 * scale),
			static_cast<int>(camera.y + y1 * scale),
			static_cast<int>(camera.x + x2 * scale),
			static_cast<int>(camera.y + y2 * scale));
	else
		result = SDL_RenderDrawLine(renderer,
			static_cast<int>(x1 * scale),
			static_cast<int>(y1 * scale),
			static_cast<int>(x2 * scale),
			static_cast<int>(y2 * scale));

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
	float scale = Engine::GetInstance().window->GetScale() * cameraZoom;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_Point points[360];

	float factor = (float)M_PI / 180.0f;

	for (int i = 0; i < 360; ++i)
	{
		int cx = static_cast<int>(x * scale);
		int cy = static_cast<int>(y * scale);
		int cr = static_cast<int>(radius * scale);

		points[i].x = (use_camera ? static_cast<int>(camera.x) : 0) + cx + static_cast<int>(cr * cos(i * factor));
		points[i].y = (use_camera ? static_cast<int>(camera.y) : 0) + cy + static_cast<int>(cr * sin(i * factor));
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

	if (cameraZoom != targetCameraZoom)
		cameraZoom += (targetCameraZoom - cameraZoom) * cameraZoomSmoothing;

	int baseW = Engine::GetInstance().window->width;
	int baseH = Engine::GetInstance().window->height;

	camera.w = static_cast<int>(baseW / cameraZoom);
	camera.h = static_cast<int>(baseH / cameraZoom);

	mapWidthPx = Engine::GetInstance().map->GetMapWidth();
	mapHeightPx = Engine::GetInstance().map->GetMapHeight();

	targetX = static_cast<int>(targetPosition.x);
	targetY = static_cast<int>(targetPosition.y);

	// Cámara look-ahead horizontal
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

	cameraLookAheadOffset = EaseInOut(cameraLookAheadOffset, cameraLookAheadTarget, lookAheadSmoothing);

	int targetCamX = -targetX + camera.w / 2 - static_cast<int>(cameraLookAheadOffset) + cameraImpulseX;
	camera.x += static_cast<int>((targetCamX - camera.x) * smoothing);

	cameraImpulseX = static_cast<int>(cameraImpulseX * (1.0f - cameraImpulseSmoothing));

	// Coordenadas de la cámara en el mundo
	int cameraTop = -camera.y;
	int cameraBottom = -camera.y + camera.h;

	// Margen de anticipación (antes de que el player se salga)
	int anticipationMargin = 75;

	float dynamicSmoothing = smoothing;

	// Si el player está cerca del borde inferior (a punto de salirse)
	if (targetY > cameraBottom - anticipationMargin) {
		dynamicSmoothing = smoothing * 2.0f; // acelerar seguimiento vertical
	}

	int targetCamY = -targetY + camera.h / 2 + cameraOffsetY;
	camera.y += static_cast<int>((targetCamY - camera.y) * dynamicSmoothing);

	// Shake
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

bool Render::DrawText(const char* text, int posx, int posy, SDL_Color color, int fontSize) const {
	TTF_Font* customFont = TTF_OpenFont("Assets/Fonts/MarkaziText-Medium.ttf", fontSize);
	if (!customFont) {
		LOG("Failed to load font: %s", TTF_GetError());
		return false;
	}

	SDL_Surface* surface = TTF_RenderText_Solid(customFont, text, color);
	if (!surface) {
		LOG("Failed to create surface: %s", TTF_GetError());
		TTF_CloseFont(customFont);
		return false;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture) {
		LOG("Failed to create texture: %s", SDL_GetError());
		SDL_FreeSurface(surface);
		TTF_CloseFont(customFont);
		return false;
	}

	int texW = 0, texH = 0;
	SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
	SDL_Rect dstrect = { posx, posy, texW, texH };
	SDL_RenderCopy(renderer, texture, NULL, &dstrect);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	TTF_CloseFont(customFont);
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
float Render::EaseInOut(float current, float target, float t)
{
	float delta = target - current;
	float easedT = t * t * (3 - 2 * t); // curva tipo easeInOutCubic
	return current + delta * easedT;
}
void Render::SetVSync(bool enable)
{
	// Save Config
	pugi::xml_document config;
	pugi::xml_parse_result result = config.load_file("config.xml");
	pugi::xml_node vSyncData = config.child("config").child("render").child("vsync");
	vSyncData.attribute("value") = enable;

	config.save_file("config.xml");
}

float Render::GetCameraZoom() const
{
	return cameraZoom;
}