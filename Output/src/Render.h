#pragma once

#include "Module.h"
#include "Vector2D.h"
#include "SDL2/SDL.h"
#include "Timer.h"
#include "SDL2/SDL_ttf.h"

class Render : public Module
{
public:

	Render();
	virtual ~Render();

	bool Awake();
	bool Start();

	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	bool CleanUp();

	void SetViewPort(const SDL_Rect& rect);
	void ResetViewPort();

	bool DrawTexture(SDL_Texture* texture, uint32_t x, uint32_t y, const SDL_Rect* section = NULL, float speed = 1.0f, double angle = 0, uint32_t pivotX = INT_MAX, uint32_t pivotY = INT_MAX, SDL_RendererFlip flip = SDL_FLIP_NONE, float scale = 1.0f) const;
	bool DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool filled = true, bool useCamera = true) const;
	bool DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;
	bool DrawCircle(int x1, int y1, int redius, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;

	void SetBackgroundColor(SDL_Color color);

	void UpdateCamera(const Vector2D& targetPosition, int movementDirection, float smoothing);
	bool DrawText(const char* text, int posx, int posy, SDL_Color color, int fontSize) const;
	int GetTextWidth(const std::string& text, int fontSize);

	float EaseInOut(float current, float target, float t);

	void SetVSync(bool enable);
	SDL_Texture* LoadTexture(const char* path);
	float GetCameraZoom() const;

	void DashCameraImpulse(int direction, int intensity);
	void StartCameraShake(int durationSec, int intensity);
	void ToggleCameraLock();

public:

	//Camera Dash
	int cameraImpulseX = 0;
	float cameraImpulseSmoothing = 0.1f;

	//Camera Movement
	int targetY;
	int targetX;
	int mapWidthPx;
	int mapHeightPx;
	int cameraLookAheadTarget = 0;
	float cameraLookAheadOffset = 0.0f;
	float lookAheadSmoothing = 0.1f;
	int lookAheadDelayFrames = 10;
	int lookAheadCounter = 0;
	int lastMoveDir = 0;
	int lookAheadDistance = 55;

	//Camera Shake
	Timer shakeTimer;
	bool isShaking = false;
	int shakeDurationSec = 0;
	int shakeIntensity = 0;
	int shakeOffsetX = 0;
	int shakeOffsetY = 0;

	bool cameraLocked = false;

	//Camera Zoom
	float cameraZoom = 1.0f;
	float targetCameraZoom = 1.0f;
	float cameraZoomSmoothing = 0.01f;

	SDL_Renderer* renderer;
	SDL_Rect camera;
	SDL_Rect viewport;
	SDL_Color background;

	TTF_Font* font;

	int cameraOffsetY = 400;
};
