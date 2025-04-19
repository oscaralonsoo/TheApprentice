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

private:
	std::string dialoguesPath = "dialogues.xml";

	std::unordered_map<int, DialogueEvent> dialogueMap;
};
