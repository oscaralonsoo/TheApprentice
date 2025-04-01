#pragma once
#include "Engine.h"
#include "Module.h"
#include <SDL2/SDL.h>
#include "Render.h"
#include <vector>
#include "GuiControlButton.h"
#include "GuiControl.h"
#include "GuiManager.h"

enum class MenusState {
    INTRO, MAINMENU, GAME, SETTINGS, CREDITS, PAUSE, NONE, EXIT
};

struct MenuButton {
    SDL_Rect rect;
    SDL_Texture* texSelected;
    SDL_Texture* texDeselected;
};

class Menus : public Module {
public:
    Menus();
    virtual ~Menus();
    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate() override;
    bool CleanUp() override;
    void LoadTextures();
    void CheckCurrentState(float dt);
    void HandlePause();
    void DrawBackground();
    void ApplyTransitionEffect();
    void StartTransition(bool fast, MenusState newState);
    void Transition(float dt);
    void Intro(float dt);
    void MainMenu(float dt);
    void NewGame();
    void Pause(float dt);
    void Settings();
    void Credits();

public:
    // Estado del menú
    MenusState currentState = MenusState::MAINMENU;
    MenusState nextState = MenusState::NONE;
    MenusState previousState = MenusState::NONE;
    bool isPaused = false;
    bool inMainMenu = false;
    bool isExit = false;
    bool inTransition = false;
    bool fastTransition = false;
    bool fadingIn = false;  
    bool inConfig = false;  
    bool inCredits = false;
    int isSaved = 0;

private:
    // Variables de transición
    float introTimer = 0.0f;
    float logoAlpha = 0.0f;
    float transitionAlpha = 0.0f; 
    float transitionSpeed = 0.0f;

    // Texturas
    SDL_Texture* textures;
    SDL_Texture* groupLogo;
    SDL_Texture* menuBackground;
    SDL_Texture* pauseBackground;
    SDL_Texture* creditsBackground;
    SDL_Texture* settingsBackground;

    // Botones
    std::vector<MenuButton> mainMenuButtons;
    std::vector<MenuButton> pauseMenuButtons;
    int selectedButton = 0;

    // Configuración de pantalla
    int width = 0, height = 0;
    bool isFullScreen = false;
};
