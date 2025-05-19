// Menus.h
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
#include <SDL2/SDL_gamecontroller.h>

enum class MenusState {
    NONE, INTRO, MAINMENU, GAME, PAUSE, SETTINGS, CREDITS, DEAD, GAMEOVER, EXIT, ABILITIES
};

struct ButtonInfo {
    std::string text;
    SDL_Rect bounds;
    int id;
    bool isCheckBox = false;
    SDL_Texture* unhoveredTexture = nullptr;
    SDL_Texture* hoveredTexture = nullptr;
    std::string unhoveredTexturePath;
    std::string hoveredTexturePath;

    ButtonInfo(const std::string& text, const SDL_Rect& bounds, int id, bool isCheckBox,
        const std::string& unhoveredPath, const std::string& hoveredPath)
        : text(text), bounds(bounds), id(id), isCheckBox(isCheckBox),
        unhoveredTexturePath(unhoveredPath), hoveredTexturePath(hoveredPath) {}
};

class Menus : public Module {
public:
    Menus();
    virtual ~Menus();

    bool Awake() override;
    bool Start() override;
    void LoadConfig();
    void LoadButtonTextures(pugi::xml_document& doc);
    void LoadCheckboxTextures(pugi::xml_document& doc);
    void LoadCheckboxTexture(pugi::xml_node node, SDL_Texture*& texture);
    bool Update(float dt) override;
    bool PostUpdate() override;
    bool CleanUp() override;

    void LoadTextures();
    void LoadAbilityTextures(pugi::xml_document& doc);
    void LoadBackgroundTextures(pugi::xml_document& doc);
    void CheckCurrentState(float dt);
    void HandlePause();
    void DrawBackground();
    std::string GetBackgroundKey() const;
    void ApplyTransitionEffect();
    void StartTransition(bool fast, MenusState newState);
    void Transition(float dt);
    void CreateButtons();
    std::vector<std::string> GetButtonNamesForCurrentState() const;
    void Intro(float dt);
    void MainMenu(float dt);
    void NewGame();
    void Pause(float dt);
    void Settings();
    void HandleSettingsSelection();
    void ToggleFullScreen();
    void ToggleVSync();
    void HandleVolumeSliders();
    void AdjustVolume(int& sliderX, int minX, int maxX);
    void UpdateVolume(int sliderX, int minX, int maxX);
    void Credits();
    void CreateButton(const std::string& name, int startX, int startY, int buttonWidth, int buttonHeight, int index);
    void DrawButtons();
    void DrawCheckBox(const ButtonInfo& button, bool isSelected);
    void DrawAbilities();
    void DrawSliders();
    void SetController(SDL_GameController* controller);
    bool ContinueLoadingScreen();
    void DrawSlider(int minX, int y, int& sliderX, bool isSelected, const std::string& label);
    void DrawPlayerLives();

public:
    MenusState currentState = MenusState::INTRO;
    MenusState nextState = MenusState::NONE;
    MenusState previousState = MenusState::NONE;

    bool isPaused = false;
    bool isExit = false;
    bool inTransition = false;
    bool fastTransition = false;
    bool fadingIn = false;
    bool inConfig = false;
    bool inCredits = false;
    bool drawingAbilityBackground = false;
    int isSaved = 0;
    int selectedButton = 0;
    std::vector<ButtonInfo> buttons;
    std::vector<std::string> buttonNames;
    std::string abilityName;
    int baseWidth, baseHeight, width, height;

    SDL_GameController* controller = nullptr;
    bool aHeld = false;
    bool startHeld = false;
    bool dpadUpHeld = false;
    bool dpadDownHeld = false;
private:
    const std::string CONFIG_FILE = "config.xml";
    const std::string ART_FILE = "art.xml";
    const std::string BACKGROUND_PATH = "art/textures/UI/menu/backgrounds";
    const std::string BUTTON_PATH = "art/textures/UI/menu/buttons";
    const std::string CHECKBOX_PATH = "art/textures/UI/menu/checkbox";

    const int VOLUME_ADJUSTMENT_STEP = 5;
    const int SLIDER_MIN = 1100;
    const int SLIDER_MAX = 1510;
    const int BUTTON_WIDTH = 200;
    const int BUTTON_HEIGHT = 15;
    const int BUTTON_SPACING = 50;

    int musicVolumeSliderX = SLIDER_MIN;
    int fxVolumeSliderX = SLIDER_MIN;
    int masterVolumeSliderX = SLIDER_MAX;

    std::unordered_map<std::string, SDL_Texture*> backgroundTextures;
    std::unordered_map<std::string, SDL_Texture*> buttonTextures;
    std::unordered_map<std::string, SDL_Texture*> loadedAbilityTextures;
    SDL_Texture* checkboxTexture = nullptr;
    SDL_Texture* fillTexture = nullptr;

    std::unordered_map<MenusState, int> previousSelectedButton; 

    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float transitionAlpha = 0.0f;
    float transitionSpeed = 0.0f;
    float logoAlpha = 0.0f;
    float introTimer = 0.0f;

    SDL_Texture* groupLogo = nullptr;
    SDL_Texture* menuBackground = nullptr;
    SDL_Texture* pauseBackground = nullptr;
    SDL_Texture* creditsBackground = nullptr;
    SDL_Texture* settingsBackground = nullptr;

    GuiManager* guiManager = nullptr;

    bool isFullScreen = false;
    bool isVSync = false;

    SDL_Color WHITE = { 255, 255, 255, 255 };
    SDL_Color GRAY = { 200, 200, 200, 255 };


    SDL_Texture* lifeTexture = nullptr;
    int lifeW = 32;
    int lifeH = 32;
};
