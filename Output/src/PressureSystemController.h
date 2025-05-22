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

    std::vector<PressurePlate*> plates;
    std::vector<PressureDoor*> doors;

private:

};
