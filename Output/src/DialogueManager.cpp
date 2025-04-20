#include "DialogueManager.h"
#include "LOG.h"
#include "Render.h"
#include "Engine.h"

DialogueManager::DialogueManager() : Module()
{
	name = "dialoguemanager";
}

// Destructor
DialogueManager::~DialogueManager()
{}

bool DialogueManager::Awake()
{
	return true;
}

bool DialogueManager::Start() {
    LoadDialogues();
	return true;
}

bool DialogueManager::Update(float dt)
{
    if (dialogueAvailable && !dialogueStarted) {
        ShowInteractionPrompt();

        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {
            dialogueStarted = true;
            currentLineIndex = 0;
        }
    }

    if (dialogueStarted && activeDialogueId != -1) {
        if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
            currentLineIndex++;
        }

        RenderDialogue(activeDialogueId);
    }

    return true;
}

bool DialogueManager::PostUpdate()
{
	return true;
}

bool DialogueManager::CleanUp()
{
	return true;
}

void DialogueManager::LoadDialogues() {
    pugi::xml_document doc;
    if (!doc.load_file(dialoguesPath.c_str())) { LOG("Error al cargar %s\n", dialoguesPath.c_str()); return; }

    for (auto node : doc.child("dialogues").children("dialogue")) {
        int id = node.attribute("id").as_int();

        DialogueEvent data;

        std::string speaker = node.child("speaker").text().as_string();
        data.speaker = speaker;

        for (auto line : node.children("line")) {
            std::string text = line.text().as_string("");
            data.lines.push_back(text);
        }

        dialogueMap[id] = data;
    }
}

void DialogueManager::RenderDialogue(int dialogueId) {
    auto it = dialogueMap.find(dialogueId);
    if (it == dialogueMap.end()) return;

    const DialogueEvent& event = it->second;

    if (currentLineIndex >= event.lines.size()) {
        dialogueStarted = false;
        currentLineIndex = 0;
        return;
    }

    SDL_Rect dialogueBox = { 500, 500, 1200, 150 };
    Engine::GetInstance().render->DrawRectangle(dialogueBox, 0, 0, 0, 180, true, true);

    int y = 100;
    Engine::GetInstance().render->DrawText(event.speaker.c_str(), 50, y, { 255, 255, 100, 255 }, 48);
    y += 50;

    Engine::GetInstance().render->DrawText(event.lines[currentLineIndex].c_str(), 50, y, { 255, 255, 255, 255 }, 40);
}

void DialogueManager::SetDialogueAvailable(int dialogueId, bool active) {
    dialogueAvailable = active;
    dialogueStarted = false;
    activeDialogueId = active ? dialogueId : -1;
}

void DialogueManager::ShowInteractionPrompt() {
    Engine::GetInstance().render->DrawText("Presiona E para hablar", 600, 400, { 255, 255, 255, 255 }, 40);
}