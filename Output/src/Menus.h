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
    NONE, INTRO, MAINMENU, GAME, PAUSE, SETTINGS, CREDITS, DEAD, GAMEOVER, EXIT
};

// Estructura para definir un botón
struct MenuButton {
    SDL_Rect bounds;
    std::string text;
    GuiControlType type;
    GuiControlButton* guiButton = nullptr;
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
    void InitializeMenus();
    bool PostUpdate() override;
    bool CleanUp() override;
    void LoadTextures();
    void CheckCurrentState(float dt);
    void HandlePause();
    void DrawBackground();
    void ApplyTransitionEffect();
    void StartTransition(bool fast, MenusState newState);
    void Transition(float dt);
    void HandleInput();
    void Intro(float dt);
    void MainMenu(float dt);
    void NewGame();
    void Pause(float dt);
    void Settings();
    void Credits();

    //Buttons
    void DrawButtons();

    void DrawTexts(MenusState state);


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
    int buttonWidth = 300;
    int centerX = 0;
    int selectedButton = 0;
    float glowEffect = 0.0f;
    bool increasingGlow = true;

    // Configuración de pantalla
    int width = 0; 
    int height = 0;
    bool isFullScreen = false;
    bool isVSync = false;

    // Colors
    SDL_Color WHITE = { 255, 255, 255, 255 };
    SDL_Color BLACK = { 0, 0, 0, 255 };
    SDL_Color RED = { 255, 0, 0, 255 };
    SDL_Color MAGENTA = { 255, 0, 255, 255 };
    SDL_Color YELLOW = { 255, 255, 0, 255 };
    SDL_Color GRAY = { 200, 200, 200, 255 };

};
