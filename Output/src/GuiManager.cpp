#include "GuiManager.h"
#include "Engine.h"
#include "Textures.h"
#include "GuiControlButton.h"
#include "Audio.h"
#include "Input.h"
#include "Menus.h"

GuiManager::GuiManager() : Module()
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

    if (type == GuiControlType::BUTTON)
    {
        guiControl = new GuiControlButton(id, bounds, text);
    }

    if (guiControl)
    {
        guiControl->observer = observer;
        guiControlsList.push_back(guiControl);
    }

    return guiControl;
}

bool GuiManager::Update(float dt)
{
    for (GuiControl* control : guiControlsList)
    {
        if (control)
            control->Update(dt);
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
            ++count;
        }
    }
    return count;
}

bool GuiManager::CleanUp()
{

    for (auto& control : guiControlsList)
    {
        delete control;
    }
    guiControlsList.clear();

    return true;
}
