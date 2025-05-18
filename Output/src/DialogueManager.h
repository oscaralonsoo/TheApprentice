#pragma once

#include "Module.h"
#include "Timer.h"
#include <unordered_map>
#include "Vector2D.h"


struct SDL_Texture;

struct DialogueEvent {
	std::string speaker;
	std::vector<std::string> lines;
	std::vector<std::vector<std::string>> wrappedLines;
};

class DialogueManager : public Module
{
public:

	DialogueManager();
	virtual ~DialogueManager();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool PostUpdate();
	bool CleanUp();

	void RenderDialogue(int id);
	void LoadDialogues();
	void SetDialogueAvailable(int dialogueId, Vector2D npcPos, bool active);
	void ShowInteractionPrompt();
	void WrapLines(int dialogueId, int boxWidth, int dialogueFontSize);
	void ResetTyping();

private:
	SDL_Texture* texture;

	std::string dialoguesPath = "dialogues.xml";

	std::unordered_map<int, DialogueEvent> dialogueMap;

	Vector2D promptPos;

	bool dialogueStarted = false;
	bool dialogueAvailable = false;
	int activeDialogueId = -1;

	int currentLineIndex = 0;
	int currentCharIndex = 0;
	Timer typingTimer;
	bool typingFinished = false;
	bool forceTypingFinish = false;
};
