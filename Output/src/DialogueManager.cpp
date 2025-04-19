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
    RenderDialogue(1);
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
    if (it != dialogueMap.end()) {
        const DialogueEvent& event = it->second;

        for (const auto& line : event.lines) {
            Engine::GetInstance().render->DrawText(line.c_str(), 0, 0, { 255, 255, 255, 255 }, 45);
        }
    }
}