#pragma once

#include "SDL2/SDL_Rect.h"
#include "pugixml.hpp"
#define MAX_FRAMES 400

class Animation {

public:
	float speed = 1.0f;
	SDL_Rect frames[MAX_FRAMES];
	bool loop = true;
	// Allows the animation to keep going back and forth
	bool pingpong = false;
	int loopCount = 0;

	float currentFrame = 0.0f;
	int totalFrames = 0;
	int pingpongDirection = 1;

	bool paused = false;

public:
	void PushBack(const SDL_Rect& rect) {
		frames[totalFrames++] = rect;
	}

	void Reset() {
		currentFrame = 0.0f;
		loopCount = 0;
		pingpongDirection = 1;
	}

	bool HasFinished() {
		return !loop && !pingpong && loopCount > 0;
	}

	void Update() {
		if (paused || totalFrames == 0) return;

		currentFrame += speed;
		if (currentFrame >= totalFrames) {
			currentFrame = (loop || pingpong) ? 0.0f : totalFrames - 1;
			++loopCount;
			if (pingpong)
				pingpongDirection = -pingpongDirection;
		}
	}

	const SDL_Rect& GetCurrentFrame() const {
		int actualFrame = static_cast<int>(currentFrame);

		if (pingpongDirection == -1) actualFrame = totalFrames - static_cast<int>(currentFrame);

		return frames[actualFrame];
	}

	void LoadAnimations(pugi::xml_node animationNode)
	{
		speed = animationNode.attribute("speed").as_float();
		loop = animationNode.attribute("loop").as_bool();

		for (pugi::xml_node animation = animationNode.child("frame"); animation; animation = animation.next_sibling("frame"))
		{
			PushBack({ animation.attribute("x").as_int(),
						animation.attribute("y").as_int(),
						animation.attribute("w").as_int(),
						animation.attribute("h").as_int() });
		}

	}

	void SetPaused(bool value) {
		paused = value;
	}

	bool IsLoop() const { return loop; }

	int GetLoopCount() const { return loopCount; }

	float GetSpeed() const { return speed; }

	int GetFrameCount() const { return totalFrames; }
};
