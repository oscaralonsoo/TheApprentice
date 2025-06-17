#include "DialogueManager.h"
#include "LOG.h"
#include "Render.h"
#include "Engine.h"
#include "Textures.h"
#include "Scene.h"

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

	listenTexture = Engine::GetInstance().textures->Load("Assets/Textures/UI/Help/listen.png");
	dialogueTexture = Engine::GetInstance().textures->Load("Assets/Textures/UI/Dialogue/dialogue.png");

	return true;
}

bool DialogueManager::PostUpdate() {
	SDL_GameController* controller = Engine::GetInstance().scene->GetPlayer()->GetMechanics()->GetMovementHandler()->GetController();
	bool l1PressedNow = false;
	bool l1JustPressed = false;

	if (controller && SDL_GameControllerGetAttached(controller)) {
		l1PressedNow = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) == 1;
		l1JustPressed = (l1PressedNow && !prevL1State);
		prevL1State = l1PressedNow;
	}

	if (dialogueAvailable && !dialogueStarted) {
		ShowInteractionPrompt();

		bool interactKeyPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_DOWN;

		if (interactKeyPressed || l1JustPressed) {
			SetPlayerMovement(true);
			dialogueStarted = true;
			currentLineIndex = 0;
			ResetTyping();
		}
	}

	if (dialogueStarted && activeDialogueId != -1) {
		bool nextKeyPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN;

		if (nextKeyPressed || l1JustPressed) {
			if (!typingFinished) {
				forceTypingFinish = true;
			}
			else {
				currentLineIndex++;
				ResetTyping();
			}
		}
		RenderDialogue(activeDialogueId);
	}

	return true;
}

bool DialogueManager::Update(float dt) {
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
		SetPlayerMovement(false);
        dialogueStarted = false;
        currentLineIndex = 0;
        return;
    }

    SDL_Rect camera = Engine::GetInstance().render->camera;

    int boxWidth = windowWidth * 0.87f;
    int boxHeight = windowHeight * 0.2f;
    int boxX = (windowWidth - boxWidth) / 2;
    int boxY = windowHeight - boxHeight - (windowHeight * 0.12f);

    SDL_Rect dialogueBox = { -camera.x + boxX, -camera.y + boxY, boxWidth, boxHeight };
	Engine::GetInstance().render->DrawTexture(dialogueTexture, dialogueBox.x, dialogueBox.y);

    int marginX = boxWidth * 0.05f;
    int marginTop = boxHeight * 0.15f;
    int lineSpacing = boxHeight * 0.21f;

    int speakerFontSize = boxHeight * 0.3f;
    int dialogueFontSize = boxHeight * 0.15f;

    int speakerX = boxX + marginX;
    int speakerY = boxY + marginTop;

    int dialogueX = boxX + marginX;
    int dialogueY = speakerY + lineSpacing;

    Engine::GetInstance().render->DrawText(event.speaker.c_str(), speakerX + 185, speakerY, { 255, 255, 255, 255 }, speakerFontSize, true);

	const std::vector<std::string>& lines = event.wrappedLines[currentLineIndex];

	float elapsedTime = typingTimer.ReadMSec();
	float typingSpeed = 15.0f;

	int totalChars = 0;
	for (const std::string& l : lines) {
		totalChars += l.length();
	}

	int charsToDisplay = forceTypingFinish ? totalChars : (int)(elapsedTime / typingSpeed);

	int displayedChars = 0;
	for (const std::string& l : lines) {
		int charsInLine = std::min((int)l.length(), charsToDisplay - displayedChars);
		std::string textToDisplay = l.substr(0, charsInLine);
		Engine::GetInstance().render->DrawText(textToDisplay.c_str(), dialogueX, dialogueY + 50, { 255, 255, 255, 255 }, dialogueFontSize, true);
		dialogueY += lineSpacing;
		displayedChars += charsInLine;

		if (displayedChars >= charsToDisplay) break;
	}

	if (displayedChars >= totalChars) {
		typingFinished = true;
	}
}

void DialogueManager::WrapLines(int dialogueId, int boxWidth, int dialogueFontSize) {
	auto& event = dialogueMap[dialogueId];
	event.wrappedLines.clear();

	int maxLineWidth = boxWidth - boxWidth * 0.02;

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

void DialogueManager::SetDialogueAvailable(int dialogueId, Vector2D npcPos, bool active) {
	dialogueAvailable = active;
	dialogueStarted = false;
	activeDialogueId = active ? dialogueId : -1;
	promptPos = npcPos;

	if (active) {
		currentLineIndex = 0;
		ResetTyping();
	}
}

void DialogueManager::ShowInteractionPrompt() {
	Engine::GetInstance().render.get()->DrawTexture(listenTexture, promptPos.x - 295/2, promptPos.y);
}

void DialogueManager::ResetTyping() {
	typingTimer.Start();
	currentCharIndex = 0;
	typingFinished = false;
	forceTypingFinish = false;
}

void DialogueManager::SetPlayerMovement(bool isMoving) {
	Player* player = Engine::GetInstance().scene.get()->GetPlayer();
	player->pbody->body->SetLinearVelocity(b2Vec2_zero);
	player->GetMechanics()->GetMovementHandler()->SetCantMove(isMoving);
	player->GetMechanics()->GetJumpMechanic()->Enable(!isMoving);
	//player->GetMechanics()->GetMovementHandler()->EnableJump(isMoving);
	//player->GetMechanics()->GetMovementHandler()->EnableDoubleJump(isMoving);
	//player->GetMechanics()->GetMovementHandler()->EnableDash(isMoving);
}