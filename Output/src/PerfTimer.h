#pragma once
#include "SDL2\SDL_timer.h"

class PerfTimer
{
public:

	// Constructor
	PerfTimer();

	void Start();
	double ReadMs() const;
	double ReadSec() const;
	Uint64 ReadTicks() const;

private:
	Uint64 startTime;
	Uint64 frequency;
};