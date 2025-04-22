#include "DialogueManager.h"
#include "LOG.h"
#include "Render.h"
#include "Engine.h"

#include <sstream> 
#include <string>
#include <vector>

DialogueManager::DialogueManager() : Module()
{
	name = "dialoguemanager";
}

DialogueManager::~DialogueManager() {}

bool DialogueManager::Awake() {
	return true;
}

bool DialogueManager::Start() {
	LoadDialogues();

	int windowWidth, windowHeight;
	SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &windowWidth, &windowHeight);
	int boxWidth = windowWidth * 0.8f;
	int dialogueFontSize = windowHeight * 0.2f * 0.15f;

	for (auto& pair : dialogueMap) {
		int id = pair.first;
		WrapLines(id, boxWidth, dialogueFontSize);
	}

	return true;
}

bool DialogueManager::Update(float dt) {
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

bool DialogueManager::PostUpdate() {
	return true;
}

bool DialogueManager::CleanUp() {
	return true;
}

void DialogueManager::LoadDialogues() {
	pugi::xml_document doc;
	if (!doc.load_file(dialoguesPath.c_str())) {
		LOG("Error al cargar %s\n", dialoguesPath.c_str());
		return;
	}

	for (auto node : doc.child("dialogues").children("dialogue")) {
		int id = node.attribute("id").as_int();

		DialogueEvent data;
		data.speaker = node.child("speaker").text().as_string();

		for (auto line : node.children("line")) {
			std::string text = line.text().as_string("");
			data.lines.push_back(text);
		}

		dialogueMap[id] = data;
	}
}

void DialogueManager::RenderDialogue(int dialogueId) {
	int windowWidth, windowHeight;
	SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &windowWidth, &windowHeight);

	auto it = dialogueMap.find(dialogueId);
	if (it == dialogueMap.end()) return;

	const DialogueEvent& event = it->second;

	if (event.wrappedLines.empty() || currentLineIndex >= event.wrappedLines.size()) {
		dialogueStarted = false;
		currentLineIndex = 0;
		return;
	}

	SDL_Rect camera = Engine::GetInstance().render->camera;

	int boxWidth = windowWidth * 0.8f;
	int boxHeight = windowHeight * 0.2f;
	int boxX = (windowWidth - boxWidth) / 2;
	int boxY = windowHeight - boxHeight - (windowHeight * 0.05f);

	SDL_Rect dialogueBox = { -camera.x + boxX, -camera.y + boxY, boxWidth, boxHeight };
	Engine::GetInstance().render->DrawRectangle(dialogueBox, 0, 0, 0, 180, true, true);

	int marginX = boxWidth * 0.05f;
	int marginTop = boxHeight * 0.15f;
	int lineSpacing = boxHeight * 0.2f;

	int speakerFontSize = boxHeight * 0.25f;
	int dialogueFontSize = boxHeight * 0.15f;

	int speakerX = boxX + marginX;
	int speakerY = boxY + marginTop;

	int dialogueX = boxX + marginX;
	int dialogueY = speakerY + lineSpacing;

	Engine::GetInstance().render->DrawText(event.speaker.c_str(), speakerX, speakerY, { 255, 255, 100, 255 }, speakerFontSize);

	// Render pre-wrapped lines
	const std::vector<std::string>& lines = event.wrappedLines[currentLineIndex];
	for (const std::string& l : lines) {
		Engine::GetInstance().render->DrawText(l.c_str(), dialogueX, dialogueY, { 255, 255, 255, 255 }, dialogueFontSize);
		dialogueY += lineSpacing;
	}
}

void DialogueManager::WrapLines(int dialogueId, int boxWidth, int dialogueFontSize) {
	auto& event = dialogueMap[dialogueId];
	event.wrappedLines.clear();

	int marginX = boxWidth * 0.05f;
	int maxLineWidth = boxWidth - 2 * marginX;

	for (const std::string& line : event.lines) {
		std::istringstream words(line);
		std::string word;
		std::string currentLine;
		std::vector<std::string> wrapped;

		while (words >> word) {
			std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
			int testWidth = Engine::GetInstance().render->GetTextWidth(testLine, dialogueFontSize);

			if (testWidth > maxLineWidth) {
				if (!currentLine.empty())
					wrapped.push_back(currentLine);
				currentLine = word;
			}
			else {
				currentLine = testLine;
			}
		}
		if (!currentLine.empty())
			wrapped.push_back(currentLine);

		event.wrappedLines.push_back(wrapped);
	}
}

void DialogueManager::SetDialogueAvailable(int dialogueId, bool active) {
	dialogueAvailable = active;
	dialogueStarted = false;
	activeDialogueId = active ? dialogueId : -1;
}

void DialogueManager::ShowInteractionPrompt() {
	Engine::GetInstance().render->DrawText("Press E to talk", 600, 400, { 255, 255, 255, 255 }, 40);
}
