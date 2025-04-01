#include "Menus.h"
#include "Scene.h"
#include "Input.h"
#include <SDL2/SDL.h>
#include "Render.h"
#include "Window.h"
#include "Engine.h"

Menus::Menus() : currentState(MenusState::MAINMENU), transitionAlpha(0.0f), inTransition(false), fadingIn(false), nextState(MenusState::NONE), 
fastTransition(false), menuBackground(nullptr), pauseBackground(nullptr) {}

Menus::~Menus() {}

bool Menus::Awake(){ return true; }

bool Menus::Start() {
    currentState = MenusState::MAINMENU;
    LoadTextures();
    InitializeMenus();

    //Load Config
    pugi::xml_document config;
    if (config.load_file("config.xml")) {
        pugi::xml_node saveData = config.child("config").child("scene").child("save_data");
        pugi::xml_node fullScreenData = config.child("config").child("window").child("fullscreen_window");
        isFullScreen = fullScreenData.attribute("value").as_bool();
        isSaved = saveData.attribute("isSaved").as_int();
    }
    return true;
}
void Menus::LoadTextures()
{
    // Cargar fondos
    groupLogo = Engine::GetInstance().render->LoadTexture("assets/textures/Menus/Logo.png");
    menuBackground = Engine::GetInstance().render->LoadTexture("assets/textures/Menus/MainMenuBackGround.png");
    pauseBackground = Engine::GetInstance().render->LoadTexture("assets/textures/Menus/PauseMenuBackground.png");
    settingsBackground = Engine::GetInstance().render->LoadTexture("assets/textures/Menus/SettingsBackground.png");
    creditsBackground = Engine::GetInstance().render->LoadTexture("assets/textures/Menus/CreditsBackground.png");
}
void Menus::InitializeMenus() {
    SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &width, &height);
    centerX = (width - buttonWidth) / 2;

    // Menú principal
    menuConfigurations[MenusState::MAINMENU] = {
        MenusState::MAINMENU, {
            {{centerX, 400, buttonWidth, 115}, "New Game", GuiControlType::BUTTON},
            {{centerX, 520, buttonWidth, 115}, "Continue", GuiControlType::BUTTON},
            {{centerX, 640, buttonWidth, 115}, "Settings", GuiControlType::BUTTON},
            {{centerX, 760, buttonWidth, 115}, "Credits", GuiControlType::BUTTON},
            {{centerX, 880, buttonWidth, 115}, "Exit", GuiControlType::BUTTON}
        }
    };

    // Menú de pausa
    menuConfigurations[MenusState::PAUSE] = {
        MenusState::PAUSE, {
            {{centerX, 400, buttonWidth, 115}, "Resume", GuiControlType::BUTTON},
            {{centerX, 520, buttonWidth, 115}, "Settings", GuiControlType::BUTTON},
            {{centerX, 640, buttonWidth, 115}, "Exit", GuiControlType::BUTTON}
        }
    };
    menuConfigurations[MenusState::SETTINGS] = {
    MenusState::SETTINGS, {
        {{centerX, 400, buttonWidth, 115}, "TogglScreen", GuiControlType::CHECKBOX},
        //Other Buttons
    }
    };
}


bool Menus::Update(float dt) {
    HandleInput();
    CheckCurrentState(dt);
    Transition(dt);
    HandlePause();
    return true;
}

// Pause Logic
void Menus::HandlePause() {
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN && !inTransition && !inConfig) {
        if (currentState == MenusState::PAUSE) {
            StartTransition(true, MenusState::GAME);
            isPaused = false;
        }
        else if (currentState == MenusState::GAME) {
            StartTransition(true, MenusState::PAUSE);
            isPaused = true;
        }
    }
}

bool Menus::PostUpdate()
{   
    DrawBackground();
    DrawButtons();
    if (inTransition) ApplyTransitionEffect();
    if (isExit) return false;
    return true;
}

void Menus::DrawBackground() // Draw background depending on the MenusState
{
    SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &width, &height);
    SDL_Rect cameraRect = { 0, 0,width, height };
    switch (currentState)
    {
    case MenusState::INTRO:
        Engine::GetInstance().render->DrawTexture(groupLogo, 0, 0, &cameraRect, logoAlpha);
        break;
        break;
    case MenusState::MAINMENU:
        Engine::GetInstance().render->DrawTexture(menuBackground, 0, 0, &cameraRect);
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
        Engine::GetInstance().render->DrawTexture(creditsBackground, 0, 0, &cameraRect);
        break;
    }
}

void Menus::ApplyTransitionEffect() {
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
        isExit = true;
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
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN)
    {
        switch (selectedButton)
        {
        case 0: // New Game
            NewGame();
            break;
        case 1: // Continue 
            if (isSaved > 0) {
                Engine::GetInstance().scene.get()->LoadGameXML();
                StartTransition(false, MenusState::GAME);
            }
            break;
        case 2: // Settings
            inConfig = true;
            StartTransition(true, MenusState::SETTINGS);
            break;
        case 3: // Credits
            inCredits = true;
            StartTransition(true, MenusState::CREDITS);
            break;
        case 4: // Exit
            currentState = MenusState::EXIT;
            break;
        }
    }
}

void Menus::NewGame()
{
    isSaved = 0;
    //Load xml
    pugi::xml_document config;
    pugi::xml_parse_result result = config.load_file("config.xml");
    pugi::xml_node saveData = config.child("config").child("scene").child("save_data");

    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();	//Reset Player Pos
    pugi::xml_node playerNode = saveData.child("player");
        playerNode.attribute("x") = 180;
        playerNode.attribute("y") = 50;

    pugi::xml_node sceneNode = saveData.child("scene"); 	
        sceneNode.attribute("actualScene") = 0; //Reset Actual Scene
        saveData.attribute("isSaved") = Engine::GetInstance().menus->isSaved;
 
    config.save_file("config.xml");	//Save Changes
 
    StartTransition(false, MenusState::GAME);
}

void Menus::Pause(float dt)
{
    if (inTransition) return;

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

    for (size_t i = 0; i < menuConfigurations[MenusState::SETTINGS].buttons.size(); ++i)
    {
        auto& button = menuConfigurations[MenusState::SETTINGS].buttons[i];
        if (button.text == "TogglScreen")
        {
            if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN))
            {
                isFullScreen = !isFullScreen;
                Engine::GetInstance().window->SetFullScreen(isFullScreen);
            }
        }
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

void Menus::Transition(float dt) // Transition Logic
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
            Engine::GetInstance().scene->saving = false;
        }
    }
}

void Menus::HandleInput() {
    if (menuConfigurations.find(currentState) == menuConfigurations.end()) return;
    size_t buttonCount = menuConfigurations[currentState].buttons.size();

    if (buttonCount == 0) return;

    Input* input = Engine::GetInstance().input.get();

    if (input->GetKey(SDL_SCANCODE_S) == KEY_DOWN) 
        selectedButton = (selectedButton + 1) % buttonCount;

    else if (input->GetKey(SDL_SCANCODE_W) == KEY_DOWN) 
        selectedButton = (selectedButton - 1 + buttonCount) % buttonCount;
}

void Menus::DrawButtons()
{
    if (menuConfigurations.find(currentState) == menuConfigurations.end()) return;
    auto& buttons = menuConfigurations[currentState].buttons;

    // Glow Effect
    if (increasingGlow) {
        glowEffect += 2.0f;
        if (glowEffect >= 150.0f) increasingGlow = false;
    }
    else {
        glowEffect -= 2.0f;
        if (glowEffect <= 40.0f) increasingGlow = true;
    }

    for (size_t i = 0; i < buttons.size(); ++i)
    {
        bool isSelected = (selectedButton == static_cast<int>(i));

        // Text Color
        SDL_Color textColor = isSelected ? SDL_Color{ 255, 255, 255, 255 } : SDL_Color{ 200, 200, 200, 255 };

        if (isSelected) {
            int glowRadius = 10;

            for (int layer = glowRadius; layer > 0; layer--) {
                Uint8 alpha = static_cast<Uint8>(glowEffect / (layer * 1.5f));
                SDL_Color glowColor = { 255, 255, 255, alpha };

                Engine::GetInstance().render->DrawText(buttons[i].text.c_str(), buttons[i].bounds.x - layer, buttons[i].bounds.y, buttons[i].bounds.w, buttons[i].bounds.h, glowColor);
                Engine::GetInstance().render->DrawText(buttons[i].text.c_str(), buttons[i].bounds.x + layer, buttons[i].bounds.y, buttons[i].bounds.w, buttons[i].bounds.h, glowColor);
                Engine::GetInstance().render->DrawText(buttons[i].text.c_str(), buttons[i].bounds.x, buttons[i].bounds.y - layer, buttons[i].bounds.w, buttons[i].bounds.h, glowColor);
                Engine::GetInstance().render->DrawText(buttons[i].text.c_str(), buttons[i].bounds.x, buttons[i].bounds.y + layer, buttons[i].bounds.w, buttons[i].bounds.h, glowColor);
            }
        }
        // Draw Text
        Engine::GetInstance().render->DrawText(buttons[i].text.c_str(), buttons[i].bounds.x, buttons[i].bounds.y, buttons[i].bounds.w, buttons[i].bounds.h, textColor);
    }
}

