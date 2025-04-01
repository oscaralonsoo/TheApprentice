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
        return false;
    }

    Input* input = Engine::GetInstance().input.get();

    static int selectedButton = 0; // Índice del botón actualmente seleccionado

    if (id == selectedButton)
    {
        state = GuiControlState::FOCUSED;
    }
    else
    {
        state = GuiControlState::NORMAL;
    }

    if (input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
    {
        selectedButton = (selectedButton - 1 + Engine::GetInstance().guiManager->GetTotalButtons()) % Engine::GetInstance().guiManager->GetTotalButtons();
    }
    else if (input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
    {
        selectedButton = (selectedButton + 1) % Engine::GetInstance().guiManager->GetTotalButtons();
    }

    if (id == selectedButton && input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN)
    {
        state = GuiControlState::PRESSED;
        NotifyObserver();
    }

    switch (state)
    {
    case GuiControlState::NORMAL:
        Engine::GetInstance().render->DrawRectangle(bounds, 200, 200, 200, 100, true, false);
        break;
    case GuiControlState::FOCUSED:
        Engine::GetInstance().render->DrawRectangle(bounds, 0, 180, 255, 100, true, false);
        break;
    case GuiControlState::PRESSED:
        Engine::GetInstance().render->DrawRectangle(bounds, 0, 255, 150, 100, true, false);
        break;
    }

    Engine::GetInstance().render->DrawText(text.c_str(), bounds.x, bounds.y, bounds.w, bounds.h);
    return false;
}

