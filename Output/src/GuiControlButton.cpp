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

	/*	switch (state)
		{
		case GuiControlState::NORMAL:
			Engine::GetInstance().render->DrawText(button.text, 200, 200, 200, 100, true, false);
			break;
		case GuiControlState::FOCUSED:

			Engine::GetInstance().render->DrawText(button.text, 0, 180, 255, 100, true, false);
			break;
		case GuiControlState::PRESSED:
			Engine::GetInstance().render->DrawText(button.text, 0, 255, 150, 100, true, false);
			break;
		case GuiControlState::DISABLED:
			Engine::GetInstance().render->DrawText(button.text, );*/

		}
    return false;
}
