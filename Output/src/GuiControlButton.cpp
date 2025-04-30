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

bool GuiControlButton::Update(float dt) {
    if (state == GuiControlState::DISABLED)
        return false;

    state = GuiControlState::FOCUSED;

    bool confirm = false;

    // --- Confirmar con teclado (ENTER) ---
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN) {
        confirm = true;
    }

    // --- Confirmar con botón A del mando ---
    Menus* menus = Engine::GetInstance().menus.get();
    SDL_GameController* controller = menus->controller;

    if (controller && SDL_GameControllerGetAttached(controller)) {
        bool aNow = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
        if (aNow && !menus->aHeld) {
            confirm = true;
        }
        menus->aHeld = aNow;
    }

    // --- Ejecutar acción si se confirma ---
    if (confirm) {
        state = GuiControlState::PRESSED;
        NotifyObserver();
    }

    return false;
}

void GuiControlButton::ButtonNavigation() {
    Menus* menus = Engine::GetInstance().menus.get();

    if (menus->currentState == MenusState::MAINMENU ||
        menus->currentState == MenusState::PAUSE ||
        menus->currentState == MenusState::SETTINGS)
    {
        auto input = Engine::GetInstance().input;
        SDL_GameController* controller = menus->controller;

        bool moveDown = input->GetKey(SDL_SCANCODE_S) == KEY_DOWN || input->GetKey(SDL_SCANCODE_DOWN) == KEY_DOWN;
        bool moveUp = input->GetKey(SDL_SCANCODE_W) == KEY_DOWN || input->GetKey(SDL_SCANCODE_UP) == KEY_DOWN;

        if (controller && SDL_GameControllerGetAttached(controller)) {
            bool dpadDown = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            bool dpadUp = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);

            if (dpadDown && !menus->dpadDownHeld) moveDown = true;
            if (dpadUp && !menus->dpadUpHeld) moveUp = true;

            menus->dpadDownHeld = dpadDown;
            menus->dpadUpHeld = dpadUp;
        }

        if (moveDown) {
            menus->selectedButton = (menus->selectedButton + 1) % menus->buttons.size();
        }
        if (moveUp) {
            menus->selectedButton = (menus->selectedButton - 1 + menus->buttons.size()) % menus->buttons.size();
        }
    }
}
