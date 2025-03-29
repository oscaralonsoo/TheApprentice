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
    groupLogo = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/Logo.png");
    menuBackground = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/MainMenuBackGround.png");
    pauseBackground = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/PauseMenu.png");
    creditsBackground = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/PauseMenu.png");
    settingsBackground = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/PauseMenu.png");

    mainMenuButtons.clear();
    pauseMenuButtons.clear();
    const int screenWidth = Engine::GetInstance().window->width;
    const int buttonWidth = 200;
    const int buttonHeight = 50;
    const int startX = (screenWidth - buttonWidth) / 2;
    const int startY = 230;
    const int spacing = 110;

    // Buttons
    mainMenuButtons.clear();
    pauseMenuButtons.clear();

    // CONTINUE
    MenuButton btnContinue;
    btnContinue.texDeselected = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/Buttons/Continue_disselected.png");
    btnContinue.texSelected = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/Buttons/Continue_Selected.png");
    btnContinue.rect = { startX, startY, buttonWidth, buttonHeight };
    mainMenuButtons.push_back(btnContinue);
    pauseMenuButtons.push_back(btnContinue);

    // SETTINGS
    MenuButton btnSettings;
    btnSettings.texDeselected = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/Buttons/Settings_disselected.png");
    btnSettings.texSelected = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/Buttons/Settings_Selected.png");
    btnSettings.rect = { startX, startY + spacing, buttonWidth, buttonHeight };
    mainMenuButtons.push_back(btnSettings);
    pauseMenuButtons.push_back(btnSettings);

    // CREDITS
    MenuButton btnCredits;
    btnCredits.texDeselected = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/Buttons/Credits_disselected.png");
    btnCredits.texSelected = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/Buttons/Credits_Selected.png");
    btnCredits.rect = { startX, startY + spacing * 2, buttonWidth, buttonHeight };
    mainMenuButtons.push_back(btnCredits);

    // EXIT
    MenuButton btnExit;
    btnExit.texDeselected = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/Buttons/Exit_disselected.png");
    btnExit.texSelected = Engine::GetInstance().render->LoadTexture("Assets/Textures/Menus/Buttons/Exit_Selected.png");

    // In MainMenu
    btnExit.rect = { startX, startY + spacing * 3, buttonWidth, buttonHeight };
    mainMenuButtons.push_back(btnExit);

    // In PauseMenu
    btnExit.rect = { startX, startY + spacing * 2, buttonWidth, buttonHeight };
    pauseMenuButtons.push_back(btnExit);
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
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE)&&!inTransition)
    {
        if (currentState == MenusState::GAME)
        {
            SetPauseTransition(true, MenusState::PAUSE);
        }
        else if (currentState == MenusState::PAUSE)
        {
            SetPauseTransition(true, MenusState::GAME);
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
        Engine::GetInstance().render->DrawTexture(settingsBackground, 0, 0, nullptr);
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
        if (logoAlpha > 1.0f) {
            logoAlpha = 1.0f;
        }
    }

    if (introTimer >= 2000.0f)  // Transition to MAINMENU after 3 seconds
    {
        fastTransition = false;
        fadingIn = false;
        inTransition = true;
        nextState = MenusState::MAINMENU;
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) // Transition to MAINMENU after SPACE is pressed 
    {
        fastTransition = false;
        fadingIn = false;
        inTransition = true;
        nextState = MenusState::MAINMENU;
    }
}

void Menus::MainMenu(float dt)
{
    // Navigation
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
    {
        selectedButton--;
        if (selectedButton < 0)
            selectedButton = mainMenuButtons.size() - 1;
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
    {
        selectedButton++;
        if (selectedButton >= mainMenuButtons.size())
            selectedButton = 0;
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN)
    {
        switch (selectedButton)
        {
        case 0: // Continue
            fastTransition = false;
            fadingIn = false;
            inTransition = true;
            nextState = MenusState::GAME;
            break;
        case 1: // Settings
            nextState = MenusState::SETTINGS;
            break;
        case 2: // Credits

            nextState = MenusState::CREDITS;
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

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
    {
        selectedButton--;
        if (selectedButton < 0)
            selectedButton = pauseMenuButtons.size() - 1;
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
    {
        selectedButton++;
        if (selectedButton >= pauseMenuButtons.size())
            selectedButton = 0;
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN)
    {
        switch (selectedButton)
        {
        case 0: // Continue
            SetPauseTransition(true, MenusState::GAME);
            break;
        case 1: // Settings
            nextState = MenusState::SETTINGS;
            break;
        case 2: // Exit
            currentState = MenusState::EXIT;
            break;
        }
    }
}


void Menus::Settings()
{

    //  TODO --- Show Settings
}

void Menus::Credits()
{
    // TODO --- Show Credits
}

void Menus::SetPauseTransition(bool fast, MenusState newState)
{
    inTransition = true;
    fadingIn = false;
    isPaused = !isPaused;
    nextState = newState;
    fastTransition = fast;
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
