#pragma once

#include "Module.h"
#include <unordered_map>

struct DialogueEvent {
	std::string speaker;
	std::vector<std::string> lines;
};

class DialogueManager : public Module
{
public:

	DialogueManager();

	// Destructor
	virtual ~DialogueManager();

	// Called before render is available
	bool Awake();

	// Called after Awake
	bool Start();

	// Called every frame
	bool Update(float dt);

	// Called after Update
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	void RenderDialogue(int id);
	void LoadDialogues();
	void SetDialogueAvailable(int dialogueId, bool active);
	void ShowInteractionPrompt();

private:
	std::string dialoguesPath = "dialogues.xml";

	std::unordered_map<int, DialogueEvent> dialogueMap;

	bool dialogueStarted = false;
	bool dialogueAvailable = false;
	int activeDialogueId = -1;

	int currentLineIndex = 0;
};
