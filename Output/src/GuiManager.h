#pragma once

#include "Module.h"
#include "GuiControl.h"

#include <vector>

class GuiManager : public Module
{
public:

	// Constructor
	GuiManager();

	// Destructor
	virtual ~GuiManager();

	// Called before the first frame
	 bool Start();

	 // Called each loop iteration
	 bool Update(float dt);

	 int GetTotalButtons() const;

	// Called before quitting
	bool CleanUp();

	// Additional methods
	GuiControl* CreateGuiControl(GuiControlType type, int id, const char* text, SDL_Rect bounds, Module* observer, SDL_Rect sliderBounds = { 0,0,0,0 });

public:

	std::vector<GuiControl*> guiControlsList;
	SDL_Texture* texture;

	bool activeDebug = false;
};

