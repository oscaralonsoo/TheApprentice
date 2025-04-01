#pragma once
#include "Engine.h"
#include "Module.h"
#include <SDL2/SDL.h>
#include "Render.h"
#include <vector>
#include "GuiControlButton.h"
#include "GuiControl.h"
#include "GuiManager.h"
#include <unordered_map>

// Enum de los estados del menú
enum class MenusState {
    NONE, INTRO, MAINMENU, GAME, PAUSE, SETTINGS, CREDITS, EXIT
};

// Estructura para definir un botón
struct MenuButton {
    SDL_Rect bounds;
    std::string text;
    GuiControlType type;
};

// Estructura para manejar los menús y sus botones
struct MenuData {
    MenusState state;
    std::vector<MenuButton> buttons;
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

    //Buttons
    void DrawButtons();


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
    std::unordered_map<MenusState, MenuData> menuConfigurations;
    GuiControlButton* guiBt;
    int selectedButton = 0;

    // Configuración de pantalla
    int width = 0, height = 0;
    bool isFullScreen = false;
};
