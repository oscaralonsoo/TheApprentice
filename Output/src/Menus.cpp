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
    currentState = MenusState::INTRO;
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
}


bool Menus::Update(float dt)
{
    CheckCurrentState(dt); // Check current menu state
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
    } // Handles Pause state change
}

bool Menus::PostUpdate()
{
    DrawBackground();

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
        Engine::GetInstance().render->DrawTexture(groupLogo, 0, 0, nullptr, logoAlpha); // Assuming DrawTexture can take alpha
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

void Menus::ApplyTransitionEffect() //Draws Transition (Render)
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
    return true;
}

void Menus::CheckCurrentState(float dt) //Checks current Screen State
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
        Engine::GetInstance().EXIT;
        break;
    }
}
void Menus::Intro(float dt)
{
    introTimer += dt;

    // Fade in the logo
    if (logoAlpha < 1.0f) {
        logoAlpha += dt * 0.5f; // Increase alpha gradually for the fade-in effect
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
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
    {
        fastTransition = false;
        fadingIn = false;
        inTransition = true;
        nextState = MenusState::GAME;
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
    {
        currentState = MenusState::EXIT;
    }
}

void Menus::NewGame()
{
    // Erase Previous Game, Create New Game
}

void Menus::Continue()
{
    // Load Previous Game
}

void Menus::Pause(float dt)
{
    if (inTransition)
    {
        Transition(dt);
    }
}

void Menus::Settings()
{
    // Show Settings
}

void Menus::Credits()
{
    // Show Credits
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
