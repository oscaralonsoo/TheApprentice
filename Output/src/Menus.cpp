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

bool Menus::Start()
{
    currentState = MenusState::MAINMENU;
    LoadTextures();
    pugi::xml_document config;
    pugi::xml_parse_result result = config.load_file("config.xml");
    pugi::xml_node saveData = config.child("config").child("scene").child("save_data");
    pugi::xml_node fullScreenData = config.child("config").child("window").child("fullscreen_window");
    isFullScreen = fullScreenData.attribute("value").as_bool();

    isSaved = saveData.attribute("isSaved").as_int();
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

    // Cargar botones dinámicamente
    std::vector<std::string> buttonNames = { "NewGame", "Continue", "Settings", "Credits", "Exit" };
    for (const auto& name : buttonNames) {
        MenuButton button;
        button.texDeselected = Engine::GetInstance().render->LoadTexture(("assets/textures/Menus/buttons/" + name + "_deselected.png").c_str());
        button.texSelected = Engine::GetInstance().render->LoadTexture(("assets/textures/Menus/buttons/" + name + "_selected.png").c_str());
        mainMenuButtons.push_back(button);
    }

    std::vector<std::string> pauseButtonNames = { "Continue", "Settings", "Exit" };
    for (const auto& name : pauseButtonNames) {
        MenuButton button;
        button.texDeselected = Engine::GetInstance().render->LoadTexture(("assets/textures/Menus/buttons/" + name + "_deselected.png").c_str());
        button.texSelected = Engine::GetInstance().render->LoadTexture(("assets/textures/Menus/buttons/" + name + "_selected.png").c_str());
        pauseMenuButtons.push_back(button);
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

    if (isExit)
    {
        return false;
    }
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

void Menus::DrawButtons()
{
    std::vector<MenuButton>* buttons = nullptr;
    if (currentState == MenusState::MAINMENU) {
        buttons = &mainMenuButtons;
    }
    else if (currentState == MenusState::PAUSE) {
        buttons = &pauseMenuButtons;
    }

    if (!buttons) return;

    int yOffset = height / 2 - buttons->size() * 50 / 2;
    for (size_t i = 0; i < buttons->size(); ++i) {
        SDL_Texture* tex = (i == selectedButton) ? (*buttons)[i].texSelected : (*buttons)[i].texDeselected;

        int xPos = width / 2 - 100;
        int yPos = yOffset + i * 100;

        if (currentState == MenusState::PAUSE) {
            xPos -= Engine::GetInstance().render->camera.x;
            yPos -= Engine::GetInstance().render->camera.y;
        }

        Engine::GetInstance().render->DrawTexture(tex, xPos, yPos);
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
    // Navigation
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
    {
        selectedButton = (selectedButton == 0) ? mainMenuButtons.size() - 1 : selectedButton - 1;
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
    {
        if (isSaved == 0 && selectedButton == 0)
        {
            selectedButton = 2; 
        }
        else
        {
            selectedButton = (selectedButton == mainMenuButtons.size() - 1) ? 0 : selectedButton + 1;
        }
    }

    if (isSaved == 0 && selectedButton == 1) {
        selectedButton = (selectedButton == 0) ? 2 : selectedButton - 1; 
    }

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
    if (playerNode) {
        playerNode.attribute("x") = 180;
        playerNode.attribute("y") = 50;
    }

    pugi::xml_node sceneNode = saveData.child("scene"); 	
    if (sceneNode) {
        sceneNode.attribute("actualScene") = 0; //Reset Actual Scene
    }
    if (saveData)
    {
        saveData.attribute("isSaved") = Engine::GetInstance().menus->isSaved;
    }
    config.save_file("config.xml");	//Save Changes

    Engine::GetInstance().scene.get()->SaveGameXML();
 
    StartTransition(false, MenusState::GAME);
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
        isFullScreen = !isFullScreen;
        Engine::GetInstance().window->SetFullScreen(isFullScreen);
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
            Engine::GetInstance().scene->saving = false;
        }
    }

}
