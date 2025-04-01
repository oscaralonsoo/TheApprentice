#include "GuiManager.h"
#include "Engine.h"
#include "Textures.h"
#include "GuiControlButton.h"
#include "Audio.h"
#include "Input.h"
#include "Menus.h"

GuiManager::GuiManager() :Module()
{
	name = "guiManager";
}

GuiManager::~GuiManager() {}

bool GuiManager::Start()
{
	return true;
}

GuiControl* GuiManager::CreateGuiControl(GuiControlType type, int id, const char* text, SDL_Rect bounds, Module* observer, SDL_Rect sliderBounds)
{
	GuiControl* guiControl = nullptr;

	switch (type)
	{
	case GuiControlType::BUTTON:
		guiControl = new GuiControlButton(id, bounds, text);
		break;
	}

	guiControl->observer = observer;
	guiControlsList.push_back(guiControl);

	return guiControl;
}

bool GuiManager::Update(float dt)
{
	static int selectedButton = 0;
	int totalButtons = GetTotalButtons();
	Input* input = Engine::GetInstance().input.get();

	if (input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
	{
		selectedButton = (selectedButton - 1 + totalButtons) % totalButtons;
	}
	else if (input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
	{
		selectedButton = (selectedButton + 1) % totalButtons;
	}

	for (int i = 0; i < guiControlsList.size(); ++i)
	{
		if (i == selectedButton)
		{
			guiControlsList[i]->state = GuiControlState::FOCUSED;
		}
		else
		{
			guiControlsList[i]->state = GuiControlState::NORMAL;
		}

		guiControlsList[i]->Update(dt);
	}

	if (input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN)
	{
		guiControlsList[selectedButton]->state = GuiControlState::PRESSED;
		guiControlsList[selectedButton]->NotifyObserver();
	}

	return true;
}

int GuiManager::GetTotalButtons() const
{
	int count = 0;
	for (const auto& control : guiControlsList)
	{
		if (control->type == GuiControlType::BUTTON)
		{
			count++;
		}
	}
	return count;
}

bool GuiManager::CleanUp()
{
	for (const auto& control : guiControlsList)
	{
		delete control;
	}
	guiControlsList.clear();

	return true;
}
