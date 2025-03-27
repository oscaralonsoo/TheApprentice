#pragma once
#include "Engine.h"
#include "Module.h"
#include <SDL2/SDL.h>
#include "Render.h"
#include <list>
#include <vector>


enum class MenusState
{
    MAINMENU,
    NEWGAME,
    CONTINUE,
    PAUSE,
    SETTINGS,
    CREDITS,
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

    bool PreUpdate();
    // Called each loop iteration
    bool Update(float dt);

    bool PostUpdate();
    // Called before quitting
    bool CleanUp();

    // Called When Game Starts
    void CheckCurrentState(float dt);

    void MainMenu();

    void NewGame();

    void Continue();

    void Pause();

    void Settings();

    void Credits();

    void Transition(float dt);

public:
    MenusState currentState;
private:
    bool inPause;
    bool inTransition;
    bool ret; 

    bool transitioning = false;

    bool fadingIn = false;

    float transitionAlpha = 0.0f;

    SDL_Texture* pauseMenuImage;
};