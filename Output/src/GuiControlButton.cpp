#include "GuiControlButton.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"
#include "Scene.h"
#include "GuiManager.h"
#include "Input.h"
#include "Menus.h"

GuiControlButton::GuiControlButton(int id, SDL_Rect bounds, const char* text) : GuiControl(GuiControlType::BUTTON, id)
{
	this->bounds = bounds;
	this->text = text;

	canClick = true;
	drawBasic = false;
}
GuiControlButton::~GuiControlButton()
{
}

bool GuiControlButton::Update(float dt)
{
	if (state == GuiControlState::DISABLED)
	{
		return false; // Sal de la función y evita actualizar lógica innecesaria
	}

	else if (state != GuiControlState::FOCUSED)
	{
		state = GuiControlState::FOCUSED;

	}

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_REPEAT)
	{
		state = GuiControlState::PRESSED;

	}

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_UP)
	{
		NotifyObserver();
	}
	else {

		}
    return false;
}
void GuiControlButton::ButtonNavigation()
{
	if (Engine::GetInstance().menus->currentState == MenusState::MAINMENU ||
		Engine::GetInstance().menus->currentState == MenusState::PAUSE ||
		Engine::GetInstance().menus->currentState == MenusState::SETTINGS)
	{
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_DOWN) {
			Engine::GetInstance().menus->selectedButton = (Engine::GetInstance().menus->selectedButton + 1) % Engine::GetInstance().menus->buttons.size();
		}
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_DOWN) {
			Engine::GetInstance().menus->selectedButton = (Engine::GetInstance().menus->selectedButton - 1 + Engine::GetInstance().menus->buttons.size()) % Engine::GetInstance().menus->buttons.size();
		}
	}
}

