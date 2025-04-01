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

	// Destructor
	virtual ~Render();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	void SetViewPort(const SDL_Rect& rect);
	void ResetViewPort();

	// Drawing
	bool DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section = NULL, float speed = 1.0f, double angle = 0, int pivotX = INT_MAX, int pivotY = INT_MAX) const;
	bool DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool filled = true, bool useCamera = true) const;
	bool DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;
	bool DrawCircle(int x1, int y1, int redius, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;

	// Set background color
	void SetBackgroundColor(SDL_Color color);

	void UpdateCamera(const Vector2D& targetPosition, int movementDirection, float smoothing);
	// To render Text
	bool DrawText(const char* text, int posx, int posy, int w, int h, SDL_Color color) const;

	float EaseInOut(float current, float target, float t);

	SDL_Texture* LoadTexture(const char* path);
public:

	//Camera Dash
	int cameraImpulseX = 0;
	float cameraImpulseSmoothing = 0.1f;

	//Camera Movement
	int targetY;
	int targetX;
	int mapWidthPx;
	int mapHeightPx;
	int followMargin = 100;
	int cameraCenterY = -camera.y + camera.h / 2;
	int cameraVerticalViewOffset = 150;
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

	//Camera Lock in the center
	bool cameraLocked = false;

	//Bajar camera al lado de bajada
	float cameraYOffset = 0.0f;
	float targetCameraYOffset = 0.0f;
	float yOffsetSmoothing = 0.05f;
	int defaultYOffset = -200;
	bool isYOffsetLocked = false;


public:

	void DashCameraImpulse(int direction, int intensity);
	void StartCameraShake(int durationSec, int intensity);
	void ToggleCameraLock();
	void ToggleVerticalOffsetLock();
	
	SDL_Renderer* renderer;
	SDL_Rect camera;
	SDL_Rect viewport;
	SDL_Color background;

	// Text
	TTF_Font* font;
};