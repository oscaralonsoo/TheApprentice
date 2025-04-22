#pragma once

#include "Module.h"
#include <unordered_map>

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
	void SetDialogueAvailable(int dialogueId, bool active);
	void ShowInteractionPrompt();
	void WrapLines(int dialogueId, int boxWidth, int dialogueFontSize);

private:
	std::string dialoguesPath = "dialogues.xml";

	std::unordered_map<int, DialogueEvent> dialogueMap;

	bool dialogueStarted = false;
	bool dialogueAvailable = false;
	int activeDialogueId = -1;

	int currentLineIndex = 0;
};
