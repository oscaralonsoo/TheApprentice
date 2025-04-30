#pragma once

#include "GuiControl.h"
#include "Vector2D.h"
#include <SDL2/SDL_gamecontroller.h>

class GuiControlButton : public GuiControl
{

public:

	GuiControlButton(int id, SDL_Rect bounds, const char* text);
	virtual ~GuiControlButton();

	// Called each loop iteration
	bool Update(float dt);

	void ButtonNavigation();



private:

	bool canClick = true;
	bool drawBasic = false;
	bool dpadUpHeld = false;
	bool dpadDownHeld = false;
	bool aHeld = false;
};

#pragma once