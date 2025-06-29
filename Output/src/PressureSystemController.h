#pragma once

#include <vector>
#include "PressurePlate.h"
#include "PressureDoor.h"

class PressurePlate;
class PressureDoor;

class PressureSystemController : public Module
{
public:

	PressureSystemController();
	virtual ~PressureSystemController();

    bool Awake() override { return true; }

    bool Start() override { return true; }

    bool Update(float dt) override { return true; }

    bool PostUpdate() override { return true; }

    bool CleanUp() override { return true; }

	void UpdateSystem();

    int GetActivePlatesCount(int id);

    void OpenDoor(int id);

    void CloseDoor(int id);

    void ResetTriggers();

    std::vector<PressurePlate*> plates;
    std::vector<PressureDoor*> doors;


private:

};
