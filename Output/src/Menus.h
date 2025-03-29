#pragma once
#include "Engine.h"
#include "Module.h"
#include <SDL2/SDL.h>
#include "Render.h"
#include <list>
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

class Menus : public Module
{
public:

    Menus();

    // Destructor
    virtual ~Menus();

    // Called before render is available
    bool Awake();

    // Called before the first frame
    bool Start();

    void LoadTextures();

    // Called each loop iteration
    bool Update(float dt);

    void HandleInput();

    bool PostUpdate();

    void DrawBackground();

    void ApplyTransitionEffect();
    // Called before quitting
    bool CleanUp();

    // Called When Game Starts
    void CheckCurrentState(float dt);

    void Intro(float dt);

    void MainMenu(float dt);

    void NewGame();

    void Continue();

    void Pause( float dt);

    void Settings();

    void Credits();

    void HandlePause();

    void SetPauseTransition(bool fast, MenusState newState);

    void Transition(float dt);


public:
    
    MenusState currentState;
    MenusState nextState;

    bool isPaused = false;
    bool inMainMenu = false;

private:

    //Flags
    bool inTransition = false;
    bool fastTransition = false;
    bool fadingIn = false;

    //Floats
    float introTimer = 0.0f;
    float logoAlpha = 0.0f;
    float transitionAlpha = 0.0f;
    float transitionSpeed = 0.0f;

    //Textures
    SDL_Texture* menuBackground = nullptr;
    SDL_Texture* pauseBackground = nullptr;
    SDL_Texture* creditsBackground = nullptr;
    SDL_Texture* settingsBackground = nullptr;
    SDL_Texture* groupLogo = nullptr;

};