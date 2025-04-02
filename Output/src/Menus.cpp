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

bool Menus::Awake() { return true; }

bool Menus::Start() {
    LoadTextures();
    CreateButtons();
    // Load Config
    pugi::xml_document config;
    if (config.load_file("config.xml")) {
        pugi::xml_node saveData = config.child("config").child("scene").child("save_data");
        pugi::xml_node fullScreenData = config.child("config").child("window").child("fullscreen_window");
        pugi::xml_node vSyncData = config.child("config").child("render").child("vsync");
        isFullScreen = fullScreenData.attribute("value").as_bool();
        isVSync = vSyncData.attribute("value").as_bool();
        isSaved = saveData.attribute("isSaved").as_int();
    }
    return true;
}

void Menus::LoadTextures() {
    groupLogo = Engine::GetInstance().render->LoadTexture("assets/textures/Menus/Logo.png");
    menuBackground = Engine::GetInstance().render->LoadTexture("assets/textures/Menus/MainMenuBackGround.png");
    pauseBackground = Engine::GetInstance().render->LoadTexture("assets/textures/Menus/PauseMenuBackground.png");
    settingsBackground = Engine::GetInstance().render->LoadTexture("assets/textures/Menus/SettingsBackground.png");
    creditsBackground = Engine::GetInstance().render->LoadTexture("assets/textures/Menus/CreditsBackground.png");
}

bool Menus::Update(float dt) {
    CheckCurrentState(dt);
    Transition(dt);
    HandlePause();
    return true;
}

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

bool Menus::PostUpdate() {
    SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &width, &height);
    DrawBackground();
    DrawButtons();
    if (inTransition) ApplyTransitionEffect();
    return !isExit;
}

void Menus::DrawBackground() {
    SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &width, &height);
    SDL_Rect cameraRect = { 0, 0, width, height };
    SDL_Texture* backgroundTexture = nullptr;

    switch (currentState) {
    case MenusState::INTRO: backgroundTexture = groupLogo; break;
    case MenusState::MAINMENU: backgroundTexture = menuBackground; break;
    case MenusState::PAUSE: backgroundTexture = pauseBackground; break;
    case MenusState::SETTINGS: backgroundTexture = settingsBackground; break;
    case MenusState::CREDITS: backgroundTexture = creditsBackground; break;
    }

    if (backgroundTexture) {
        Engine::GetInstance().render->DrawTexture(backgroundTexture, cameraRect.x - Engine::GetInstance().render->camera.x, cameraRect.y - Engine::GetInstance().render->camera.y, &cameraRect);
    }
}

void Menus::ApplyTransitionEffect() {
    SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, static_cast<Uint8>(transitionAlpha * 255));
    SDL_RenderFillRect(Engine::GetInstance().render->renderer, nullptr);
}

bool Menus::CleanUp() {
    SDL_DestroyTexture(groupLogo);
    SDL_DestroyTexture(menuBackground);
    SDL_DestroyTexture(pauseBackground);
    SDL_DestroyTexture(settingsBackground);
    SDL_DestroyTexture(creditsBackground);
    return true;
}

void Menus::CheckCurrentState(float dt) {
    if (inTransition) return;
    Engine::GetInstance().guiManager->controlButton->ButtonNavigation();

    switch (currentState) {
    case MenusState::INTRO: Intro(dt); break;
    case MenusState::MAINMENU: MainMenu(dt); break;
    case MenusState::GAME: break;
    case MenusState::PAUSE: Pause(dt); break;
    case MenusState::SETTINGS: Settings(); break;
    case MenusState::CREDITS: Credits(); break;
    case MenusState::EXIT: isExit = true; break;
    }
}

void Menus::Intro(float dt) {
    introTimer += dt;
    logoAlpha = std::min(logoAlpha + dt * 0.5f, 1.0f);

    if (introTimer >= 2000.0f || Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
        StartTransition(false, MenusState::MAINMENU);
    }
}

void Menus::MainMenu(float dt) {
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN) {
        switch (selectedButton) {
        case 0: NewGame(); break;
        case 1: if (isSaved > 0) { Engine::GetInstance().scene.get()->LoadGameXML(); StartTransition(false, MenusState::GAME); } break;
        case 2: inConfig = true; StartTransition(true, MenusState::SETTINGS); break;
        case 3: inCredits = true; StartTransition(true, MenusState::CREDITS); break;
        case 4: currentState = MenusState::EXIT; break;
        }
    }
}

void Menus::NewGame() {
    isSaved = 0;
    pugi::xml_document config;
    config.load_file("config.xml");
    auto saveData = config.child("config").child("scene").child("save_data");

    saveData.child("player").attribute("x") = 180;
    saveData.child("player").attribute("y") = 50;
    saveData.child("scene").attribute("actualScene") = 0;
    saveData.attribute("isSaved") = isSaved;

    config.save_file("config.xml");
    StartTransition(false, MenusState::GAME);
}

void Menus::Pause(float dt) {
    if (inTransition) return;

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN) {
        switch (selectedButton) {
        case 0: isPaused = false; StartTransition(true, MenusState::GAME); break;
        case 1: inConfig = true; StartTransition(true, MenusState::SETTINGS); break;
        case 2: currentState = MenusState::EXIT; break;
        }
    }
}

void Menus::Settings() {
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) {
        nextState = (previousState == MenusState::PAUSE) ? MenusState::PAUSE : previousState;
        inConfig = false;
        StartTransition(true, nextState);
        previousState = MenusState::NONE;
    }
    else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN) {
        switch (selectedButton) {
        case 0: isFullScreen = !isFullScreen; Engine::GetInstance().window->SetFullScreen(isFullScreen); break;
        case 1: isVSync = !isVSync; Engine::GetInstance().render->SetVSync(isVSync); break;
        }
        CreateButtons(); // Actualiza los botones después de cambiar el estado
    }
}

void Menus::Credits() {
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) {
        nextState = (previousState == MenusState::PAUSE) ? MenusState::PAUSE : previousState;
        inCredits = false;
        StartTransition(true, nextState);
        previousState = MenusState::NONE;
    }
}

void Menus::StartTransition(bool fast, MenusState newState) {
    previousState = currentState;
    fastTransition = fast;
    fadingIn = false;
    inTransition = true;
    nextState = newState;
}

void Menus::Transition(float dt) {
    transitionSpeed = fastTransition ? 0.007f : 0.0015f;

    if (!fadingIn) { // Fade Out
        transitionAlpha += dt * transitionSpeed;
        if (transitionAlpha >= 1.0f) {
            transitionAlpha = 1.0f;
            fadingIn = true;
            if (nextState != MenusState::NONE) {
                currentState = nextState;
                nextState = MenusState::NONE;
                CreateButtons();
            }
        }
    }
    else { // Fade In
        transitionAlpha -= dt * transitionSpeed;
        if (transitionAlpha <= 0.0f) {
            transitionAlpha = 0.0f;
            inTransition = false;
            fastTransition = false;
            Engine::GetInstance().scene->saving = false;
        }
    }
}
void Menus::CreateButtons() {
    Engine::GetInstance().window->GetWindowSize(baseWidth, baseHeight); SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &width, &height);

    scaleX = static_cast<float>(width) / baseWidth;
    scaleY = static_cast<float>(height) / baseHeight;

    int buttonWidth = static_cast<int>(300 * scaleX);
    int buttonHeight = static_cast<int>(50 * scaleY);
    int numButtons = (currentState == MenusState::MAINMENU) ? 5 : (currentState == MenusState::PAUSE) ? 3 : 2;

    int centerX = (width - buttonWidth) / 2;
    int startY = height / 2 - (buttonHeight * numButtons / 2) + 100;

    buttons.clear();
    std::vector<ButtonInfo> buttonConfig;

    if (currentState == MenusState::MAINMENU) {
        buttonConfig = {
            { "New Game", { centerX + 25, startY, buttonWidth - 50, buttonHeight }, 0 },
            { "Continue", { centerX + 75, startY + buttonHeight, buttonWidth - 150, buttonHeight }, 1 },
            { "Settings", { centerX + 75, startY + buttonHeight * 2, buttonWidth - 150, buttonHeight }, 2 },
            { "Credits", { centerX + 100, startY + buttonHeight * 3, buttonWidth - 200, buttonHeight }, 3 },
            { "Exit", { centerX + 150, startY + buttonHeight * 4, buttonWidth - 275, buttonHeight }, 4 }
        };
    }
    else if (currentState == MenusState::PAUSE) {
        buttonConfig = {
            { "Resume", { centerX + 100, startY - 300, buttonWidth - 200, buttonHeight }, 0 },
            { "Settings", { centerX + 75, startY - 250 + buttonHeight, buttonWidth - 150, buttonHeight }, 1 },
            { "Exit", { centerX + 150, startY - 200 + buttonHeight * 2, buttonWidth - 275, buttonHeight }, 2 }
        };
    }
    else if (currentState == MenusState::SETTINGS) {
        buttonConfig = {
            { "FullScreen", { centerX - 100, startY - 250, buttonWidth, buttonHeight }, 0, true },
            { "VSync", { centerX, startY - 250 + buttonHeight, buttonWidth - 200, buttonHeight }, 1, true }
        };
    }

    for (const auto& button : buttonConfig) {
        buttons.push_back(button);
        Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, button.id, button.text.c_str(), button.bounds, this);
    }
}

void Menus::DrawButtons() {
    if (currentState == MenusState::MAINMENU || currentState == MenusState::PAUSE || currentState == MenusState::SETTINGS) {
        for (size_t i = 0; i < buttons.size(); ++i) {
            bool isSelected = (i == selectedButton);
            SDL_Color color = isSelected ? WHITE : GRAY;

            Engine::GetInstance().render->DrawText(buttons[i].text.c_str(), buttons[i].bounds.x, buttons[i].bounds.y, buttons[i].bounds.w, buttons[i].bounds.h, color);

            if (buttons[i].isCheckBox) {
                DrawCheckBox(buttons[i], isSelected, color);
            }
        }
    }
}

void Menus::DrawCheckBox(const ButtonInfo& button, bool isSelected, const SDL_Color& color) {
    int boxSize = static_cast<int>(30 * scaleX);
    int borderThickness = static_cast<int>(isSelected ? 6 * scaleX : 4 * scaleX);
    SDL_Rect boxRect = {
        button.bounds.x + 100 + button.bounds.w - boxSize - static_cast<int>(10 * scaleX),
        button.bounds.y + (button.bounds.h - boxSize) / 2,
        boxSize,
        boxSize
    };

    SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, color.r, color.g, color.b, 255);
    for (int j = 0; j < borderThickness; ++j) {
        SDL_Rect outerBoxRect = { boxRect.x - j, boxRect.y - j, boxRect.w + 2 * j, boxRect.h + 2 * j };
        SDL_RenderDrawRect(Engine::GetInstance().render->renderer, &outerBoxRect);
    }

    if ((button.id == 0 && isFullScreen) || (button.id == 1 && isVSync)) {
        float scaleFactor = isSelected ? 0.7f : 0.5f;
        int innerSize = static_cast<int>(boxSize * scaleFactor);
        int offset = (boxSize - innerSize) / 2;

        SDL_Rect checkMark = {
            boxRect.x + offset,
            boxRect.y + offset,
            innerSize,
            innerSize
        };

        SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, color.r, color.g, color.b, 255);
        SDL_RenderFillRect(Engine::GetInstance().render->renderer, &checkMark);
    }
}