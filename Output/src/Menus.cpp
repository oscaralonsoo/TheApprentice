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
#include "Audio.h"
#include "MenuParticle.h"
#include "ParticleManager.h"

template<typename T>
T Clamp(T value, T min, T max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

Menus::Menus() : currentState(MenusState::INTRO), transitionAlpha(0.0f), inTransition(false), fadingIn(false), nextState(MenusState::NONE),
fastTransition(false), menuBackground(nullptr), pauseBackground(nullptr) {}

Menus::~Menus() {}

bool Menus::Awake() { return true; }

bool Menus::Start() {
    SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &width, &height);
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
    // Cargar la textura del slider
    pugi::xml_node sliderNode = doc.child("art").child("textures").child("UI").child("menu").child("slider").child("sliderBox");
    if (sliderNode) {
        std::string path = std::string(sliderNode.attribute("path").value()) + sliderNode.attribute("name").value();
        sliderTexture = Engine::GetInstance().render->LoadTexture(path.c_str());
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
    if (currentState == MenusState::MAINMENU)
    {
        SpawnMenuParticles();
        UpdateMenuParticles();
    }
    else {
        DestroyAllParticles();
    }
    if (inTransition) ApplyTransitionEffect();
    return !isExit;

}
void Menus::DrawBackground() {
    std::string bgKey = GetBackgroundKey();
    auto it = backgroundTextures.find(bgKey);
    if (it != backgroundTextures.end() && it->second) {
        int texW, texH;
        SDL_QueryTexture(it->second, NULL, NULL, &texW, &texH);

        SDL_Rect destRect = { 0, 0, width, height };
        SDL_RenderCopy(Engine::GetInstance().render->renderer, it->second, nullptr, &destRect);
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
    static bool menuMusicStarted = false;

    if (!menuMusicStarted) {
        Engine::GetInstance().audio->PlayMusic("Assets/Audio/music/mainmenu_music.ogg", 2.0f, 1.0f); 
        menuMusicStarted = true;
    }
    bool buttonPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN;

    if (controller && SDL_GameControllerGetAttached(controller)) {
        bool aNow = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
        if (aNow && !aHeld) {
            buttonPressed = true;
        }
        aHeld = aNow;
    }

    if (buttonPressed) {
        switch (selectedButton) {
        case 0: NewGame(); break;
        case 1: if (isSaved != 0) {
            Engine::GetInstance().scene.get()->LoadGameXML();
            Engine::GetInstance().scene->isLoading = true;

            int sceneIndex = Engine::GetInstance().scene->nextScene; 

            switch (sceneIndex) {
            case 69:
                Engine::GetInstance().audio->PlayMusic("Assets/Audio/music/palo.wav", 2.0f, 1.0f);
                break;
            case 1:
                Engine::GetInstance().audio->PlayMusic("Assets/Audio/music/cave_music.ogg", 2.0f, 1.0f);
                break;
            case 21:
                Engine::GetInstance().audio->PlayMusic("Assets/Audio/music/manglar_music.ogg", 2.0f, 1.0f);
                break;
            case 41:
                Engine::GetInstance().audio->PlayMusic("Assets/Audio/music/snowforest_music.ogg", 2.0f, 1.0f);
                break;
            case 46:
                Engine::GetInstance().audio->PlayMusic("Assets/Audio/music/snowforest_music.ogg", 2.0f, 1.0f);
                break;
            default:
                Engine::GetInstance().audio->PlayMusic("Assets/Audio/music/cave_music.ogg", 2.0f, 1.0f); 
                break;
            }

            StartTransition(false, MenusState::GAME);
        }
              break;
        case 2: inConfig = true; StartTransition(true, MenusState::SETTINGS); break;
        case 3: inCredits = true; StartTransition(true, MenusState::CREDITS); break;
        case 4: currentState = MenusState::EXIT; break;
        }
    }
}
void Menus::NewGame() {
    isSaved = 0;

    // Cargar documento XML
    pugi::xml_document config;
    config.load_file(CONFIG_FILE.c_str());

    auto saveData = config.child("config").child("scene").child("save_data");

    auto playerNode = saveData.child("player");
    playerNode.attribute("x") = 1152;
    playerNode.attribute("y") = 970;
    playerNode.attribute("lives") = 2;
    playerNode.attribute("maxlives") = 3;

    auto abilitiesNode = saveData.child("abilities");
    abilitiesNode.attribute("jump") = false;
    abilitiesNode.attribute("doublejump") = false;
    abilitiesNode.attribute("dash") = false;
    abilitiesNode.attribute("glide") = false;
    abilitiesNode.attribute("walljump") = false;
    abilitiesNode.attribute("hook") = false;
    abilitiesNode.attribute("push") = false;

    auto sceneNode = saveData.child("scene");
    if (sceneNode) {
        sceneNode.attribute("actualScene") = 1;
    }
    saveData.attribute("isSaved") = isSaved;

    config.save_file(CONFIG_FILE.c_str());

    Engine::GetInstance().audio->PlayMusic("Assets/Audio/music/cave_music.ogg", 2.0f, 1.0f);

    StartTransition(false, MenusState::GAME);
}

void Menus::Pause(float dt) {
    if (inTransition) return;

    bool selectPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN;
    bool backPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN;

    if (controller && SDL_GameControllerGetAttached(controller)) {
        bool aNow = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
        bool bNow = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);

        if (aNow && !aHeld) selectPressed = true;
        if (bNow && !bHeld) backPressed = true;

        aHeld = aNow;
        bHeld = bNow;
    }

    if (backPressed) {
        isPaused = false;
        StartTransition(true, MenusState::GAME);
        return;
    }

    if (selectPressed) {
        switch (selectedButton) {
        case 0: isPaused = false; StartTransition(true, MenusState::GAME); break;
        case 1: inConfig = true; StartTransition(true, MenusState::SETTINGS); break;
        case 2: currentState = MenusState::EXIT; break;
        }
    }
}

void Menus::Settings() {
    bool escapePressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN;

    if (controller && SDL_GameControllerGetAttached(controller)) {
        bool aNow = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
        bool bNow = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);

        if (bNow && !bHeld) {
            nextState = (previousState == MenusState::PAUSE) ? MenusState::PAUSE : previousState;
            inConfig = false;
            StartTransition(true, nextState);
        }

        if (aNow && !aHeld) {
            HandleSettingsSelection();
        }

        aHeld = aNow;
        bHeld = bNow;
    }

    if (escapePressed) {
        nextState = (previousState == MenusState::PAUSE) ? MenusState::PAUSE : previousState;
        inConfig = false;
        StartTransition(true, nextState);
    }

    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN) {
        HandleSettingsSelection();
    }

    HandleVolumeSliders();
}

void Menus::HandleSettingsSelection() {
    switch (selectedButton) {
    case 0: ToggleFullScreen(); break;
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
    auto input = Engine::GetInstance().input;

    bool moveLeft = input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT ||
        input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT;

    bool moveRight = input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT ||
        input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT;

    if (controller && SDL_GameControllerGetAttached(controller)) {
        Sint16 axisX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        const Sint16 DEADZONE = 8000;  
        if (axisX < -DEADZONE && !dpadLeftHeld) {
            moveLeft = true;
            dpadLeftHeld = true;
        }
        else if (axisX > DEADZONE && !dpadRightHeld) {
            moveRight = true;
            dpadRightHeld = true;
        }

        if (axisX > -DEADZONE && axisX < DEADZONE) {
            dpadLeftHeld = false;
            dpadRightHeld = false;
        }

        if (!dpadLeftHeld && SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
            moveLeft = true;
            dpadLeftHeld = true;
        }
        if (!dpadRightHeld && SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
            moveRight = true;
            dpadRightHeld = true;
        }
    }
    if (moveLeft) {
        sliderX -= VOLUME_ADJUSTMENT_STEP;
        sliderX = std::max(sliderX, minX);
    }

    if (moveRight) {
        sliderX += VOLUME_ADJUSTMENT_STEP;
        sliderX = std::min(sliderX, maxX);
    }
    UpdateVolume(sliderX, minX, maxX);
}
void Menus::UpdateVolume(int sliderX, int minX, int maxX) {
    int squareWidth = 20; 
    int minXreal = minX + squareWidth / 2;
    int maxXreal = maxX - squareWidth / 2;

    float volume = (float)(sliderX - minXreal) / (maxXreal - minXreal);
    volume = Clamp(volume, 0.0f, 1.0f);

    auto& audio = *Engine::GetInstance().audio;

    if (selectedButton == 2) {
        audio.musicVolume = volume;
    }
    else if (selectedButton == 3) {
        audio.sfxVolume = volume;
    }
    else if (selectedButton == 4) {
        audio.masterVolume = volume;
    }

    int finalMusicVolume = static_cast<int>(audio.musicVolume * audio.masterVolume * MIX_MAX_VOLUME);
    int finalSfxVolume = static_cast<int>(audio.sfxVolume * audio.masterVolume * MIX_MAX_VOLUME);

    Mix_VolumeMusic(finalMusicVolume);
    Mix_Volume(-1, finalSfxVolume);
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
void Menus::SyncSlidersWithVolumes() {
    int minX = (width / 2) + 50;
    int maxX = minX + 420;

    auto CalcSliderX = [&](float volume, bool isSelected) -> int {
        int squareWidth = isSelected ? 25 : 20;
        int minXreal = minX + squareWidth / 2;
        int maxXreal = maxX - squareWidth / 2;
        return minXreal + static_cast<int>(volume * (maxXreal - minXreal));
        };

    musicVolumeSliderX = CalcSliderX(Engine::GetInstance().audio->musicVolume, selectedButton == 2);
    fxVolumeSliderX = CalcSliderX(Engine::GetInstance().audio->sfxVolume, selectedButton == 3);
    masterVolumeSliderX = CalcSliderX(Engine::GetInstance().audio->masterVolume, selectedButton == 4);
}
void Menus::CreateButtons() {
    buttons.clear();
    std::vector<std::string> names = GetButtonNamesForCurrentState();

    SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &width, &height);
    float scale = std::min(static_cast<float>(width) / Engine::GetInstance().window->width, static_cast<float>(height) / Engine::GetInstance().window->height);

    int buttonWidth = static_cast<int>(BUTTON_WIDTH * scale);
    int buttonHeight = static_cast<int>(BUTTON_HEIGHT * scale);
    if (currentState == MenusState::SETTINGS) {
        SyncSlidersWithVolumes();
    }
    int spacing = static_cast<int>(BUTTON_SPACING * scale);
    int totalHeight = names.size() * (buttonHeight + spacing) - spacing;

    int startX = (width - buttonWidth) / 2 - 20;

    int startY;
    if (currentState == MenusState::PAUSE) {
        spacing = static_cast<int>(BUTTON_SPACING * scale * 1.5f);
        startY = (height - totalHeight) / 2 - static_cast<int>(20 * scale); 
    }
    else if (currentState == MenusState::SETTINGS) {
        startY = (height - totalHeight) / 2 + static_cast<int>(40 * scale) + 25; 
        startX = (width - buttonWidth) / 2 - 100;
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

    bool spacePressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN;
    bool aPressed = false;

    if (controller && SDL_GameControllerGetAttached(controller)) {
        bool aNow = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
        if (aNow && !aHeld) {
            aPressed = true;
        }
        aHeld = aNow;
    }

    if (spacePressed || aPressed) {
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

    int textOffsetX = -150;
    int textOffsetY = -100;
    int textX = button.bounds.x + textOffsetX;
    int textY = button.bounds.y + (button.bounds.h / 2) - 10 + textOffsetY;

    Engine::GetInstance().render->DrawText(button.text.c_str(), textX, textY, WHITE, 45, true);
    DrawSliders();
}

void Menus::DrawSliders() {
    const int startY = (height / 2) + 100;
    const int spacing = 125;

    DrawSlider((width / 2) + 50, startY + spacing * 0, musicVolumeSliderX, selectedButton == 2, "Music Volume");
    DrawSlider((width / 2) + 50, startY + spacing * 1, fxVolumeSliderX, selectedButton == 3, "FX Volume");
    DrawSlider((width / 2) + 50, startY + spacing * 2, masterVolumeSliderX, selectedButton == 4, "Master Volume");
}

void Menus::DrawSlider(int minX, int y, int& sliderX, bool isSelected, const std::string& label) {
    auto& render = Engine::GetInstance().render;
    int cameraX = render->camera.x;
    int cameraY = render->camera.y;

    const int sliderWidth = 420;

    int squareWidth = isSelected ? 25 : 20;
    int squareHeight = isSelected ? 45 : 35;
    int halfSquare = squareWidth / 2;

    sliderX = Clamp(sliderX, minX + halfSquare, minX + sliderWidth - halfSquare);

    render->DrawRectangle({ minX, y, sliderWidth, 19 }, 200, 200, 200, 255, true, false);

    int squareX = sliderX - halfSquare - cameraX;
    int squareY = y - (squareHeight / 2) - cameraY;

    if (sliderTexture) {
        SDL_Rect srcRect = { 0, 0, squareWidth, squareHeight };
        render->DrawTexture(sliderTexture, squareX, squareY+7, &srcRect);
    }
    else {
        SDL_Rect fallback = { squareX, squareY, squareWidth, squareHeight };
        render->DrawRectangle(fallback, 100, 100, 100, 255, true, true);
    }
    int textWidth = render->GetTextWidth(label, 45);
    int textX = minX - 455;
    int textY = y - 20;
    render->DrawText(label.c_str(), textX, textY, WHITE, 45, true);
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

void Menus::SpawnMenuParticles() {
    if (rand() % 100 < 10)
    {
        int windowWidth, windowHeight;
        SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &windowWidth, &windowHeight);

        SDL_Rect camera = Engine::GetInstance().render->camera;

        int randX = rand() % windowWidth;
        int randY = rand() % windowHeight;

        MenuParticle* particle = new MenuParticle();;

        if (particle)
        {
            particle->Start();
            particle->SetPosition({ (float)randX - camera.x, (float)randY - camera.y });
            menuParticles.push_back(particle);
        }
    }
}

void Menus::UpdateMenuParticles() {
    for (MenuParticle* particle : menuParticles) {
        particle->Update();
    }
}

void Menus::DestroyAllParticles() {

    for (auto it = menuParticles.begin(); it != menuParticles.end(); ) {
        delete* it;
        it = menuParticles.erase(it);
    }
}

void Menus::DestroyMenuParticle(MenuParticle* particle) {
    for (auto it = menuParticles.begin(); it != menuParticles.end();)
    {
        if (*it == particle) {
            (*it)->CleanUp();
            delete* it;
            it = menuParticles.erase(it);
            break;
        }
        else {
            ++it;
        }
    }
}

