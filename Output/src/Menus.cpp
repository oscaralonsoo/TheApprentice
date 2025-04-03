#include "Menus.h"
#include "Scene.h"
#include "Input.h"
#include <SDL2/SDL.h>
#include "Render.h"
#include "Window.h"
#include "Engine.h"
#include "pugixml.hpp"
#include "Log.h"

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
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("art.xml");
    if (!result) {
        LOG("Error cargando art.xml: %s", result.description());
        return;
    }

    // Cargar fondos
    pugi::xml_node backgrounds = doc.child("art").child("textures").child("UI").child("menu").child("backgrounds");
    for (pugi::xml_node bg = backgrounds.first_child(); bg; bg = bg.next_sibling()) {
        std::string name = bg.name();
        std::string path = std::string(bg.attribute("path").value()) + bg.attribute("name").value();
        backgroundTextures[name] = Engine::GetInstance().render->LoadTexture(path.c_str());
    }

    // Cargar botones
    pugi::xml_node buttonsNode = doc.child("art").child("textures").child("UI").child("menu").child("buttons");
    for (pugi::xml_node btn = buttonsNode.first_child(); btn; btn = btn.next_sibling()) {
        std::string path = std::string(btn.attribute("path").value()) + btn.attribute("name").value();
        buttonTextures[btn.attribute("name").value()] = Engine::GetInstance().render->LoadTexture(path.c_str());
    }

    // Cargar checkbox
    pugi::xml_node checkboxNode = doc.child("art").child("textures").child("UI").child("menu").child("checkbox");
    if (checkboxNode) {
        pugi::xml_node checkboxImg = checkboxNode.child("checkbox");
        pugi::xml_node fillImg = checkboxNode.child("fill");

        if (checkboxImg && fillImg) {
            std::string checkboxPath = std::string(checkboxImg.attribute("path").value()) +
                checkboxImg.attribute("name").value();
            std::string fillPath = std::string(fillImg.attribute("path").value()) +
                fillImg.attribute("name").value();

            checkboxTexture = Engine::GetInstance().render->LoadTexture(checkboxPath.c_str());
            fillTexture = Engine::GetInstance().render->LoadTexture(fillPath.c_str());

            LOG("Checkbox texture loaded: %s -> %p", checkboxPath.c_str(), checkboxTexture);
            LOG("Fill texture loaded: %s -> %p", fillPath.c_str(), fillTexture);
        }
    }
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

    std::string bgKey;
    switch (currentState) {
    case MenusState::INTRO: bgKey = "intro"; break;
    case MenusState::MAINMENU: bgKey = "main"; break;
    case MenusState::PAUSE: bgKey = "pause"; break;
    case MenusState::SETTINGS: bgKey = "settings"; break;
    case MenusState::CREDITS: bgKey = "credits"; break;
    }

    auto it = backgroundTextures.find(bgKey);
    if (it != backgroundTextures.end()) {
        SDL_Texture* bgTexture = it->second;
        if (bgTexture) {
            Engine::GetInstance().render->DrawTexture(bgTexture, 
                cameraRect.x - Engine::GetInstance().render->camera.x, 
                cameraRect.y - Engine::GetInstance().render->camera.y,
                &cameraRect);
        }
    }
}

void Menus::ApplyTransitionEffect() {
    SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, static_cast<Uint8>(transitionAlpha * 255));
    SDL_RenderFillRect(Engine::GetInstance().render->renderer, nullptr);
}

bool Menus::CleanUp() {
    for (auto& pair : backgroundTextures) {
        SDL_DestroyTexture(pair.second);
    }
    for (auto& pair : buttonTextures) {
        SDL_DestroyTexture(pair.second);
    }

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
        CreateButtons(); 
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
    buttons.clear();
    std::vector<std::string> names;

    switch (currentState) {
    case MenusState::MAINMENU:
        names = { "newGame", "continue", "settings", "credits", "exit" };
        break;
    case MenusState::PAUSE:
        names = { "continue", "settings", "exit" };
        break;
    case MenusState::SETTINGS:
        names = { "fullscreen", "vsync" };
        break;
    default:
        return;
    }

    SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &width, &height);

    float scaleX = static_cast<float>(width) / Engine::GetInstance().window->width;
    float scaleY = static_cast<float>(height) / Engine::GetInstance().window->height;
    float scale = std::min(scaleX, scaleY);

    int buttonWidth = static_cast<int>(200 * scale);
    int buttonHeight = static_cast<int>(15 * scale);
    int spacing = static_cast<int>(70 * scale);
    int totalHeight = names.size() * (buttonHeight + spacing) - spacing;

    int startX = (width - buttonWidth) / 2;
    int startY = (height - totalHeight) / 2 + static_cast<int>(50 * scale);

    if (currentState == MenusState::PAUSE) {
        startY -= static_cast<int>(150 * scale);
    }

    for (size_t i = 0; i < names.size(); ++i) {
        std::string unhovered = names[i] + "_unhovered.png";
        std::string hovered = names[i] + "_hovered.png";

        SDL_Rect bounds = {
            startX,
            startY + static_cast<int>(i * (buttonHeight + spacing)),
            buttonWidth,
            buttonHeight
        };

        bool isCheckBox = (currentState == MenusState::SETTINGS);
        buttons.emplace_back(names[i], bounds, static_cast<int>(i), isCheckBox, unhovered, hovered);

        if (!isCheckBox) {
            buttons.back().unhoveredTexture = buttonTextures[unhovered];
            buttons.back().hoveredTexture = buttonTextures[hovered];
        }
    }
}

void Menus::DrawButtons() {
    for (size_t i = 0; i < buttons.size(); ++i) {
        ButtonInfo& button = buttons[i];

        if (button.isCheckBox) {
            DrawCheckBox(button, i == selectedButton);
        }
        else {
            SDL_Texture* tex = (i == selectedButton) ? button.hoveredTexture : button.unhoveredTexture;
            if (tex) {
                Engine::GetInstance().render->DrawTexture(
                    tex,
                    button.bounds.x - Engine::GetInstance().render->camera.x,
                    button.bounds.y - Engine::GetInstance().render->camera.y
                );
            }
        }
    }
}

void Menus::DrawCheckBox(const ButtonInfo& button, bool isSelected) {
 
    if (isSelected)
    {
        Engine::GetInstance().render->DrawTexture(checkboxTexture, 100, 100); // mas grande
    }
    else {
        Engine::GetInstance().render->DrawTexture(checkboxTexture, 100, 100); // mas pequeña
    }

    //si esta marcado
    Engine::GetInstance().render->DrawTexture(fillTexture,100,100);
}

