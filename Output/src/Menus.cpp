#include "Menus.h"
#include "Scene.h"
#include "Input.h"
#include <SDL2/SDL.h>
#include "Render.h"
#include "Window.h"

Menus::Menus() : currentState(MenusState::MAINMENU), transitionAlpha(0.0f), inTransition(false), fadingIn(false), nextState(MenusState::NONE), 
fastTransition(false), menuBackground(nullptr), pauseBackground(nullptr) {}

Menus::~Menus() {}

bool Menus::Awake(){ return true; }

bool Menus::Start()
{
    currentState = MenusState::MAINMENU;
    LoadTextures();
    return true;
}

void Menus::LoadTextures()
{
    //Load Background Textures
    groupLogo = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/Logo.png");
    menuBackground = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/MainMenuBackground.png");
    pauseBackground = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/PauseMenuBackground.png");
    creditsBackground = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/CreditsBackground.png");
    settingsBackground = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/SettingsBackground.png");


    const int screenWidth = Engine::GetInstance().window->width;
    const int buttonWidth = 200;
    const int buttonHeight = 50;
    const int startX = (screenWidth - buttonWidth) / 2;
    const int startY = 230;
    const int buttonSpacing = 110;    


    mainMenuButtons.clear();
    pauseMenuButtons.clear();


    std::vector<std::tuple<std::string, int>> buttonData = {
        {"Continue", startY},
        {"Settings", startY + buttonSpacing},
        {"Credits", startY + buttonSpacing * 2},
        {"Exit", startY + buttonSpacing * 3}
    };

    // Create Buttons
    for (size_t i = 0; i < buttonData.size(); ++i)
    {
        const std::string& buttonName = std::get<0>(buttonData[i]);
        const int posY = std::get<1>(buttonData[i]);

        MenuButton btn;
        btn.texDeselected = Engine::GetInstance().render->LoadTexture(("Assets/Textures/Menus/Buttons/" + buttonName + "_disselected.png").c_str());
        btn.texSelected = Engine::GetInstance().render->LoadTexture(("Assets/Textures/Menus/Buttons/" + buttonName + "_Selected.png").c_str());
        btn.rect = { startX, posY, buttonWidth, buttonHeight };

        mainMenuButtons.push_back(btn);
        if (buttonName != "Credits") 
            pauseMenuButtons.push_back(btn);
    }

    for (size_t i = 0; i < pauseMenuButtons.size(); ++i)
    {
        pauseMenuButtons[i].rect.y = startY + (i * buttonSpacing); 
    }
}

bool Menus::Update(float dt)
{
    CheckCurrentState(dt);
    Transition(dt);
    HandlePause();
    return true;
}
// Pause Logic
void Menus::HandlePause()
{
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN && !inTransition && !inConfig)
    {
        if (currentState == MenusState::PAUSE)
        {
            StartTransition(true, MenusState::GAME);
            isPaused = false;
        }
        else if (currentState == MenusState::GAME)
        {
            StartTransition(true, MenusState::PAUSE);
            isPaused = true;
        }
    }
}

bool Menus::PostUpdate()
{
    DrawBackground();

    DrawButtons();

    if (inTransition)
        ApplyTransitionEffect();
    return true;
}

void Menus::DrawBackground() // Draw background depending on the MenusState
{
    SDL_Rect cameraRect = { 0, 0,Engine::GetInstance().window.get()->width,Engine::GetInstance().window.get()->height };
    switch (currentState)
    {
    case MenusState::INTRO:
        Engine::GetInstance().render->DrawTexture(groupLogo, 0, 0, nullptr, logoAlpha);
        break;
        break;
    case MenusState::MAINMENU:
        Engine::GetInstance().render->DrawTexture(menuBackground, 0, 0, nullptr);
        break;
    case MenusState::PAUSE:
        Engine::GetInstance().render->DrawTexture(pauseBackground, cameraRect.x - Engine::GetInstance().render->camera.x, 
            cameraRect.y - Engine::GetInstance().render->camera.y, &cameraRect);
        break;
    case MenusState::SETTINGS:
        Engine::GetInstance().render->DrawTexture(settingsBackground, cameraRect.x - Engine::GetInstance().render->camera.x,
            cameraRect.y - Engine::GetInstance().render->camera.y, &cameraRect);
        break;
    case MenusState::CREDITS:
        Engine::GetInstance().render->DrawTexture(creditsBackground, 0, 0, nullptr);
        break;
    }
}

void Menus::DrawButtons()
{
    std::vector<MenuButton>* buttons = nullptr;

    if (currentState == MenusState::MAINMENU)
        buttons = &mainMenuButtons;
    else if (currentState == MenusState::PAUSE)
        buttons = &pauseMenuButtons;

    if (buttons)
    {
        for (size_t i = 0; i < buttons->size(); i++)
        {
            SDL_Texture* tex = (i == selectedButton) ? (*buttons)[i].texSelected : (*buttons)[i].texDeselected;

            int adjustedX = (*buttons)[i].rect.x;
            int adjustedY = (*buttons)[i].rect.y;

            if (currentState == MenusState::PAUSE)  
            {
                adjustedX -= Engine::GetInstance().render->camera.x;
                adjustedY -= Engine::GetInstance().render->camera.y;
            }

            Engine::GetInstance().render->DrawTexture(tex, adjustedX, adjustedY, nullptr);
        }
    }
}

void Menus::ApplyTransitionEffect()
{
    SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, static_cast<Uint8>(transitionAlpha * 255));
    SDL_RenderFillRect(Engine::GetInstance().render->renderer, nullptr);
}

bool Menus::CleanUp()
{
    SDL_DestroyTexture(groupLogo);
    SDL_DestroyTexture(menuBackground);
    SDL_DestroyTexture(pauseBackground);
    SDL_DestroyTexture(settingsBackground);
    SDL_DestroyTexture(creditsBackground);

    for (auto& btn : mainMenuButtons)
    {
        SDL_DestroyTexture(btn.texDeselected);
        SDL_DestroyTexture(btn.texSelected);
    }
    return true;
}

void Menus::CheckCurrentState(float dt) 
{
    if (inTransition) return;

    switch (currentState)
    {
    case MenusState::INTRO:
        Intro(dt);
        break;
    case MenusState::MAINMENU:
        MainMenu(dt);
        break;
    case MenusState::GAME:
        break;
    case MenusState::NEWGAME:
        NewGame();
        break;
    case MenusState::CONTINUE:
        Continue();
        break;
    case MenusState::PAUSE:
        Pause(dt);
        break;
    case MenusState::SETTINGS:
        Settings();
        break;
    case MenusState::CREDITS:
        Credits();
        break;
    case MenusState::EXIT:
        exit(1);
        break;
    }
}

void Menus::Intro(float dt)
{
    introTimer += dt;

    // Fade in the logo
    if (logoAlpha < 1.0f) {
        logoAlpha += dt * 0.5f; 
        if (logoAlpha > 1.0f) logoAlpha = 1.0f;
    }

    if (introTimer >= 2000.0f || Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
    {
        StartTransition(false, MenusState::MAINMENU);
    }
}

void Menus::MainMenu(float dt)
{
    // Navigation
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
    {
        selectedButton = (selectedButton == 0) ? mainMenuButtons.size() - 1 : selectedButton - 1;
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
    {
        selectedButton = (selectedButton == mainMenuButtons.size() - 1) ? 0 : selectedButton + 1;
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN)
    {
        switch (selectedButton)
        {
        case 0: // Continue
            StartTransition(true, MenusState::GAME);
            break;
        case 1: // Settings
            inConfig = true;
            StartTransition(true, MenusState::SETTINGS);
            break;
        case 2: // Credits
            inCredits = true;
            StartTransition(true, MenusState::CREDITS);
            break;
        case 3: // Exit
            currentState = MenusState::EXIT;
            break;
        }
    }
}

void Menus::NewGame()
{
    // TODO --- New Game
}

void Menus::Continue()
{
    // TODO --- Load Previous Game
}

void Menus::Pause(float dt)
{
    if (inTransition) return;

    // Navigation for Pause Menu
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
    {
        selectedButton = (selectedButton == 0) ? pauseMenuButtons.size() - 1 : selectedButton - 1;
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
    {
        selectedButton = (selectedButton == pauseMenuButtons.size() - 1) ? 0 : selectedButton + 1;
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN)
    {
        switch (selectedButton)
        {
        case 0: // Continue
            isPaused = false;
            StartTransition(true, MenusState::GAME);
            break;
        case 1: // Settings
            inConfig = true;
            StartTransition(true,MenusState::SETTINGS);
            break;
        case 2: // Exit
            currentState = MenusState::EXIT;
            break;
        }
    }
}


void Menus::Settings()
{
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
    {

        if (previousState == MenusState::PAUSE)
        {
            nextState = MenusState::PAUSE;
        }
        else
        {
            nextState = previousState; 
        }
        inConfig = false;
        StartTransition(true, nextState);
        previousState = MenusState::NONE; 
    }
}

void Menus::Credits()
{
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
    {

        if (previousState == MenusState::PAUSE)
        {
            nextState = MenusState::PAUSE;
        }
        else
        {
            nextState = previousState;
        }
        inCredits = false;
        StartTransition(true, nextState);
        previousState = MenusState::NONE;
    }
}
void Menus::StartTransition(bool fast, MenusState newState)
{
    previousState = currentState; 
    fastTransition = fast;
    fadingIn = false;
    inTransition = true;
    nextState = newState;
}

// Transition Logic
void Menus::Transition(float dt)
{
    transitionSpeed = fastTransition ? 0.007f : 0.0015f;

    if (!fadingIn) // Fade Out
    {
        transitionAlpha += dt * transitionSpeed;
        if (transitionAlpha >= 1.0f)
        {
            transitionAlpha = 1.0f;
            fadingIn = true;
            if (nextState != MenusState::NONE)
            {
                currentState = nextState;
                nextState = MenusState::NONE;
            }
        }
    }
    else // Fade In
    {
        transitionAlpha -= dt * transitionSpeed;
        if (transitionAlpha <= 0.0f)
        {
            transitionAlpha = 0.0f;
            inTransition = false;
            fastTransition = false; 
        }
    }
}
