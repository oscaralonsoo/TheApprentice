#include "Menus.h"
#include "Scene.h"
#include "Input.h"
#include <SDL2/SDL.h>
#include "Render.h"
#include "Window.h"
#include "Engine.h"
#include "pugixml.hpp"
#include "Log.h"
#include <SDL2/SDL_mixer.h>
#include "Textures.h"



Menus::Menus() : currentState(MenusState::MAINMENU), transitionAlpha(0.0f), inTransition(false), fadingIn(false), nextState(MenusState::NONE),
fastTransition(false), menuBackground(nullptr), pauseBackground(nullptr) {}

Menus::~Menus() {}

bool Menus::Awake() { return true; }

bool Menus::Start() {
    LoadTextures();
    CreateButtons();
    LoadConfig();
    return true;
}
void Menus::LoadConfig() {
    pugi::xml_document config;
    if (config.load_file(CONFIG_FILE.c_str())) {
        isFullScreen = config.child("config").child("window").child("fullscreen_window").attribute("value").as_bool();
        isVSync = config.child("config").child("render").child("vsync").attribute("value").as_bool();
        isSaved = config.child("config").child("scene").child("save_data").attribute("isSaved").as_int();
    }
}
void Menus::LoadTextures() {
    pugi::xml_document doc;
    if (doc.load_file(ART_FILE.c_str())) {
        LoadBackgroundTextures(doc);
        LoadButtonTextures(doc);
        LoadCheckboxTextures(doc);
        LoadAbilityTextures(doc);
    }
    lifeTexture = Engine::GetInstance().textures->Load("Assets/Slime/vida_slime.png");
}
void Menus::LoadAbilityTextures(pugi::xml_document& doc) {
    for (pugi::xml_node ability = doc.child("art").child("textures").child("UI").child("menu").child("backgrounds").child("jump"); ability; ability = ability.next_sibling()) {
        std::string name = ability.name();
        std::string path = std::string(ability.attribute("path").value()) + ability.attribute("name").value();
        loadedAbilityTextures[name] = Engine::GetInstance().render->LoadTexture(path.c_str());
    }
}
void Menus::LoadBackgroundTextures(pugi::xml_document& doc) {
    for (pugi::xml_node bg = doc.child("art").child("textures").child("UI").child("menu").child("backgrounds").first_child(); bg; bg = bg.next_sibling()) {
        std::string name = bg.name();
        std::string path = std::string(bg.attribute("path").value()) + bg.attribute("name").value();
        backgroundTextures[name] = Engine::GetInstance().render->LoadTexture(path.c_str()); 
    }
}
void Menus::LoadButtonTextures(pugi::xml_document& doc) {
    for (pugi::xml_node btn = doc.child("art").child("textures").child("UI").child("menu").child("buttons").first_child(); btn; btn = btn.next_sibling()) {
        std::string path = std::string(btn.attribute("path").value()) + btn.attribute("name").value();
        buttonTextures[btn.attribute("name").value()] = Engine::GetInstance().render->LoadTexture(path.c_str()); 
    }
}
void Menus::LoadCheckboxTextures(pugi::xml_document& doc) {
    pugi::xml_node checkboxNode = doc.child("art").child("textures").child("UI").child("menu").child("checkbox");
    if (checkboxNode) {
        LoadCheckboxTexture(checkboxNode.child("checkbox"), checkboxTexture);
        LoadCheckboxTexture(checkboxNode.child("fill"), fillTexture);
    }
}
void Menus::LoadCheckboxTexture(pugi::xml_node node, SDL_Texture*& texture) {
    if (node) {
        std::string path = std::string(node.attribute("path").value()) + node.attribute("name").value();
        texture = Engine::GetInstance().render->LoadTexture(path.c_str());
        LOG("Checkbox texture loaded: %s -> %p", path.c_str(), texture);
    }
}

bool Menus::Update(float dt) {
    CheckCurrentState(dt);
    Transition(dt);
    HandlePause();
    return true;
}
void Menus::HandlePause() {
    bool pausePressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN;

    if (controller && SDL_GameControllerGetAttached(controller)) {
        bool startNow = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START);
        if (startNow && !startHeld) {
            pausePressed = true;
        }
        startHeld = startNow;
    }

    if (pausePressed && !inTransition && !inConfig && currentState == MenusState::GAME) {
        StartTransition(true, MenusState::PAUSE);
    }
}
bool Menus::PostUpdate() {
    SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &width, &height);
    DrawBackground();
    DrawButtons();
    ContinueLoadingScreen();
    if (currentState == MenusState::ABILITIES) DrawAbilities();
    if (currentState == MenusState::GAME) DrawPlayerLives();
    if (inTransition) ApplyTransitionEffect();
    return !isExit;
}
void Menus::DrawBackground() {
    SDL_Rect cameraRect = { 0, 0, width, height };
    std::string bgKey = GetBackgroundKey();
    auto it = backgroundTextures.find(bgKey);
    if (it != backgroundTextures.end() && it->second) {
        Engine::GetInstance().render->DrawTexture(it->second, cameraRect.x - Engine::GetInstance().render->camera.x, cameraRect.y - Engine::GetInstance().render->camera.y, &cameraRect);
    }
}
std::string Menus::GetBackgroundKey() const {
    switch (currentState) {
    case MenusState::INTRO: return "intro";
    case MenusState::MAINMENU: return "main";
    case MenusState::PAUSE: return "pause";
    case MenusState::SETTINGS: return "settings";
    case MenusState::CREDITS: return "credits";
    case MenusState::ABILITIES: return "abilities";
    default: return "";
    }
}
void Menus::ApplyTransitionEffect() {
    SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, static_cast<Uint8>(transitionAlpha * 255));
    SDL_RenderFillRect(Engine::GetInstance().render->renderer, nullptr);
}
bool Menus::CleanUp() {
    for (auto& pair : backgroundTextures) { SDL_DestroyTexture(pair.second); }
    for (auto& pair : buttonTextures) { SDL_DestroyTexture(pair.second); }
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
        case 1: if (isSaved != 0) { Engine::GetInstance().scene.get()->LoadGameXML(); StartTransition(false, MenusState::GAME); 
                                    Engine::GetInstance().scene->isLoading = true; } break;
        case 2: inConfig = true; StartTransition(true, MenusState::SETTINGS); break;
        case 3: inCredits = true; StartTransition(true, MenusState::CREDITS); break;
        case 4: currentState = MenusState::EXIT; break;
        }
    }
}
void Menus::NewGame() {
    isSaved = 0;
    pugi::xml_document config;
    config.load_file(CONFIG_FILE.c_str());
    auto saveData = config.child("config").child("scene").child("save_data");

    saveData.child("player").attribute("x") = 180;
    saveData.child("player").attribute("y") = 50;
    saveData.child("scene").attribute("actualScene") = 0;
    saveData.attribute("isSaved") = isSaved;

    config.save_file(CONFIG_FILE.c_str());
    pugi::xml_node sceneNode = saveData.child("scene");
    if (sceneNode) {
        sceneNode.attribute("actualScene") = 0;
    }
    if (saveData) {
        saveData.attribute("isSaved") = Engine::GetInstance().menus->isSaved;
    }
    config.save_file("config.xml");

    Engine::GetInstance().scene.get()->SaveGameXML();

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
    }
    else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN) {
        HandleSettingsSelection();
    }
    HandleVolumeSliders();
}
void Menus::HandleSettingsSelection() {
    switch (selectedButton) {
    case 0: /*ToggleFullScreen(); */break;
    case 1: ToggleVSync(); break;
    }
}
void Menus::ToggleFullScreen() {
    isFullScreen = !isFullScreen;
    Engine::GetInstance().window->SetFullScreen(isFullScreen);
}
void Menus::ToggleVSync() {
    isVSync = !isVSync;
    Engine::GetInstance().render->SetVSync(isVSync);
}
void Menus::HandleVolumeSliders() {
    int minX = (width / 2) + 50;
    int maxX = minX + 420;

    if (selectedButton == 2) {
        AdjustVolume(musicVolumeSliderX, minX, maxX);
    }
    else if (selectedButton == 3) {
        AdjustVolume(fxVolumeSliderX, minX, maxX);
    }
    else if (selectedButton == 4) {
        AdjustVolume(masterVolumeSliderX, minX, maxX);
    }
}
void Menus::AdjustVolume(int& sliderX, int minX, int maxX) {
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
        sliderX -= VOLUME_ADJUSTMENT_STEP;
        sliderX = std::max(sliderX, minX);
    }
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
        sliderX += VOLUME_ADJUSTMENT_STEP;
        sliderX = std::min(sliderX, maxX);
    }
    UpdateVolume(sliderX, minX, maxX);
}
void Menus::UpdateVolume(int sliderX, int minX, int maxX) {
    float volume = (float)(sliderX - minX) / (maxX - minX);
    volume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;
    int sdlVolume = static_cast<int>(volume * MIX_MAX_VOLUME);
    if (selectedButton == 2) {
        Mix_VolumeMusic(sdlVolume);
    }
    else if (selectedButton == 3) {
        Mix_Volume(-1, sdlVolume);
    }
    else if (selectedButton == 4) {
        Mix_Volume(-1, sdlVolume);
        Mix_VolumeMusic(sdlVolume);
    }
}
void Menus::Credits() {
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) {
        nextState = (previousState == MenusState::PAUSE) ? MenusState::PAUSE : previousState;
        inCredits = false;
        StartTransition(true, nextState);
    }
}
void Menus::StartTransition(bool fast, MenusState newState) {
    previousState = currentState;
    fastTransition = fast;
    fadingIn = false;
    inTransition = true;
    nextState = newState;
    previousSelectedButton[currentState] = selectedButton;
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
                if (currentState == MenusState::GAME) {
                    buttons.clear();
                }
                else {
                    CreateButtons();
                }
                if (currentState == MenusState::SETTINGS) {
                    selectedButton = 0; 
                }
                else if (previousSelectedButton.find(currentState) != previousSelectedButton.end()) {
                    selectedButton = previousSelectedButton[currentState];
                }
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
            Engine::GetInstance().scene->isLoading = false;
        }
    }
}
void Menus::CreateButtons() {
    buttons.clear();
    std::vector<std::string> names = GetButtonNamesForCurrentState();

    SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &width, &height);
    float scale = std::min(static_cast<float>(width) / Engine::GetInstance().window->width, static_cast<float>(height) / Engine::GetInstance().window->height);

    int buttonWidth = static_cast<int>(BUTTON_WIDTH * scale);
    int buttonHeight = static_cast<int>(BUTTON_HEIGHT * scale);
    int spacing = static_cast<int>(BUTTON_SPACING * scale);
    int totalHeight = names.size() * (buttonHeight + spacing) - spacing;

    int startX = (width - buttonWidth) / 2 - 20;

    int startY;
    if (currentState == MenusState::PAUSE) {
        spacing = static_cast<int>(BUTTON_SPACING * scale * 1.5f);
        startY = (height - totalHeight) / 2 - static_cast<int>(20 * scale); 
    }
    else if (currentState == MenusState::SETTINGS) {
        startY = (height - totalHeight) / 2 + static_cast<int>(40 * scale); 
        spacing = static_cast<int>(BUTTON_SPACING * scale * 1.5f); 
    }
    else {
        startY = (height - totalHeight) / 2 + static_cast<int>(120 * scale); 
    }

    for (size_t i = 0; i < names.size(); ++i) {
        CreateButton(names[i], startX, startY + static_cast<int>(i * (buttonHeight + spacing)), buttonWidth, buttonHeight, i);
    }
}

std::vector<std::string> Menus::GetButtonNamesForCurrentState() const {
    switch (currentState) {
    case MenusState::MAINMENU: return { "newGame", "continue", "settings", "credits", "exit" };
    case MenusState::PAUSE: return { "continue", "settings", "exit" };
    case MenusState::SETTINGS: return { "Full Screen", "Vsync", "Music Volume", "FX Volume", "Master Volume" };
    default: return {};
    }
}

void Menus::CreateButton(const std::string& name, int startX, int startY, int buttonWidth, int buttonHeight, int index) {
    std::string unhovered = name + "_unhovered.png";
    std::string hovered = name + "_hovered.png";

    SDL_Rect bounds = { startX, startY, buttonWidth, buttonHeight };
    bool isCheckBox = (currentState == MenusState::SETTINGS && (index == 0 || index == 1));
    buttons.emplace_back(name, bounds, index, isCheckBox, unhovered, hovered);

    if (!isCheckBox) {
        buttons.back().unhoveredTexture = buttonTextures[unhovered];
        buttons.back().hoveredTexture = buttonTextures[hovered];
    }
}
void Menus::DrawAbilities() {
    if (drawingAbilityBackground == true) return;
    SDL_Texture* abilityTexture = nullptr;
    SDL_Rect cameraRect = { 0, 0, width, height };
    auto it = loadedAbilityTextures.find(abilityName);
    if (it != loadedAbilityTextures.end()) {
        abilityTexture = it->second;
    }

    if (abilityTexture) {
        Engine::GetInstance().render->DrawTexture(abilityTexture, cameraRect.x - Engine::GetInstance().render->camera.x, cameraRect.y - Engine::GetInstance().render->camera.y, &cameraRect);
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
        StartTransition(false, MenusState::GAME);
        drawingAbilityBackground = false;
    }
}
void Menus::DrawButtons() {
    if (currentState == MenusState::GAME) return;
    for (size_t i = 0; i < buttons.size(); ++i) {
        ButtonInfo& button = buttons[i];
        if (button.isCheckBox) {
            DrawCheckBox(button, i == selectedButton);
        }
        else {
            SDL_Texture* tex = (i == selectedButton) ? button.hoveredTexture : button.unhoveredTexture;
            if (tex) {
                Engine::GetInstance().render->DrawTexture(tex, button.bounds.x - Engine::GetInstance().render->camera.x, button.bounds.y - Engine::GetInstance().render->camera.y);
            }
        }
    }
}
void Menus::DrawCheckBox(const ButtonInfo& button, bool isSelected) {
    int baseSize = 60;
    float scale = isSelected ? 1.4f : 1.1f;
    int size = static_cast<int>(baseSize * scale);

    int checkboxOffsetX = 175;
    int checkboxOffsetY = -75;
    int x = button.bounds.x + button.bounds.w / 2 - size / 2 + checkboxOffsetX;
    int y = button.bounds.y + button.bounds.h / 2 - size / 2 + checkboxOffsetY;
    SDL_Rect dstRect = { x, y, size, size };
    SDL_RenderCopy(Engine::GetInstance().render->renderer, checkboxTexture, nullptr, &dstRect);

    if ((button.text == "Full Screen" && isFullScreen) || (button.text == "Vsync" && isVSync)) {
        SDL_RenderCopy(Engine::GetInstance().render->renderer, fillTexture, nullptr, &dstRect);
    }

    int textOffsetX = -100;
    int textOffsetY = -100;
    int textX = button.bounds.x + textOffsetX;
    int textY = button.bounds.y + (button.bounds.h / 2) - 10 + textOffsetY;

    Engine::GetInstance().render->DrawText(button.text.c_str(), textX, textY, WHITE, 45);
    DrawSliders();
}

void Menus::DrawSliders() {
    DrawSlider((width/2) + 50, (height/2), musicVolumeSliderX, selectedButton == 2, "Music Volume");
    DrawSlider((width / 2) + 50, (height / 2)+100, fxVolumeSliderX, selectedButton == 3, "FX Volume");
    DrawSlider((width / 2) + 50, (height / 2)+200, masterVolumeSliderX, selectedButton == 4, "Master Volume");
}

void Menus::DrawSlider(int minX, int y, int& sliderX, bool isSelected, const std::string& label) {
    // Dibuja el fondo del slider
    Engine::GetInstance().render->DrawRectangle({ minX, y, 420, 19 }, 200, 200, 200, 255, true, false);

    int squareWidth = isSelected ? 25 : 20;
    int squareHeight = isSelected ? 45 : 35;
    int squareColor = isSelected ? 255 : 150;

    // Limitar el valor de sliderX dentro de los l�mites del slider
    sliderX = std::max(minX + squareWidth / 2, std::min(sliderX, minX + 420 - squareWidth / 2));

    // Calcula la posici�n del cuadrado del slider para centrarlo
    int squareX = sliderX - (squareWidth / 2);
    int squareY = y - (squareHeight - 19) / 2; 
    Engine::GetInstance().render->DrawRectangle({ squareX, squareY, squareWidth, squareHeight },
        squareColor, squareColor, squareColor, 255, true, false);

    // Calcular la posici�n del texto a la izquierda del slider
    int textWidth = Engine::GetInstance().render->GetTextWidth(label, 45);
    int textX = minX - textWidth - 50; 
    Engine::GetInstance().render->DrawText(label.c_str(), textX, y - 20, WHITE, 45);
}
void Menus::DrawPlayerLives() {
    if (currentState != MenusState::GAME) return;
    if (!lifeTexture) return;

    Player* player = Engine::GetInstance().scene->GetPlayer();
    if (!player) return;

    int lives = player->GetMechanics()->GetHealthSystem()->GetLives();

    const int marginLeft = 100;
    const int marginTop = 60;
    const int spacing = 20;
    const int lifeW = 32;
    const int lifeH = 32;

    for (int i = 0; i < lives; ++i) {
        int x = marginLeft + i * (lifeW + spacing);
        int y = marginTop;
        SDL_Rect section = { 0, 0, lifeW, lifeH };
        Engine::GetInstance().render->DrawTexture(lifeTexture, x, y, &section, 0.0f);
    }
}
bool Menus::ContinueLoadingScreen()
{
    if (Engine::GetInstance().scene->isLoading) {
        // Aplica fade out progresivo a negro
        if (transitionAlpha < 1.0f) {
            transitionAlpha += 0.01f;
        }
        else {
            transitionAlpha = 1.0f;
        }
        SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, static_cast<Uint8>(transitionAlpha * 255));
        SDL_RenderFillRect(Engine::GetInstance().render->renderer, nullptr);
        return !isExit;
    }
}

void Menus::SetController(SDL_GameController* controller) {
    this->controller = controller;
}

