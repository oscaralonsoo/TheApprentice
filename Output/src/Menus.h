#pragma once
#include "Engine.h"
#include "Module.h"
#include <SDL2/SDL.h>
#include "Render.h"
#include <vector>

enum class MenusState
{
    INTRO,
    MAINMENU,
    NEWGAME,
    CONTINUE,
    PAUSE,
    SETTINGS,
    CREDITS,
    GAME,
    NONE,
    EXIT
};

struct MenuButton {
    SDL_Rect rect;             
    SDL_Texture* texSelected;   
    SDL_Texture* texDeselected; 
};

class Menus : public Module
{
public:
    Menus();
    virtual ~Menus();
    bool Awake();
    bool Start();
    void LoadTextures();
    bool Update(float dt);
    bool PostUpdate();
    void DrawBackground();
    void DrawButtons();
    void ApplyTransitionEffect();
    bool CleanUp();
    void CheckCurrentState(float dt);
    void Intro(float dt);
    void MainMenu(float dt);
    void NewGame();
    void Continue();
    void Pause(float dt);
    void Settings();
    void Credits();
    void HandlePause();
    void SetPauseTransition(bool fast, MenusState newState);
    void Transition(float dt);

public:

    //Menu States
    MenusState currentState;
    MenusState nextState;
    bool isPaused = false;
    bool inMainMenu = false;

private:

    //Flags
    bool inTransition = false;
    bool fastTransition = false;
    bool fadingIn = false;

    // Transitions
    float introTimer = 0.0f;
    float logoAlpha = 0.0f;
    float transitionAlpha = 0.0f;
    float transitionSpeed = 0.0f;

    // textures
    SDL_Texture* menuBackground = nullptr;
    SDL_Texture* pauseBackground = nullptr;
    SDL_Texture* creditsBackground = nullptr;
    SDL_Texture* settingsBackground = nullptr;
    SDL_Texture* groupLogo = nullptr;

    //Buttons
    std::vector<MenuButton> mainMenuButtons;
    std::vector<MenuButton> pauseMenuButtons;
    int selectedButton = 0; 
};
