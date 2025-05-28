#include "PressureSystemController.h"
#include <unordered_map>
#include "LOG.h"

PressureSystemController::PressureSystemController()
{
}

PressureSystemController::~PressureSystemController() {}

void PressureSystemController::UpdateSystem()
{
    std::unordered_map<int, int> totalPlates;
    std::unordered_map<int, int> activePlates;

    for (auto* plate : plates)
    {
        totalPlates[plate->id]++;
        if (plate->IsActive())
        {
            activePlates[plate->id]++;
        }
    }

    for (auto* door : doors)
    {
        int total = totalPlates[door->id];
        int active = activePlates[door->id];

        bool shouldOpen = (total > 0 && active == total);
        door->shouldBeOpen = shouldOpen;
        door->SetOpen(shouldOpen);
    }
}


int PressureSystemController::GetActivePlatesCount(int id)
{
    int count = 0;
    for (auto* plate : plates)
    {
        if (plate->id == id && plate->IsActive())
        {
            ++count;
        }
    }
    return count;
}
void PressureSystemController::OpenDoor(int id)
{
    for (auto* door : doors)
    {
        if (door->id == id)
        {
            door->shouldBeOpen = true;
            door->SetOpen(true);
            door->state = PressureDoorState::DISABLE; // Fuerza cambio inmediato
            break;
        }
    }
}

void PressureSystemController::CloseDoor(int id)
{
    for (auto* door : doors)
    {
        if (door->id == id)
        {
            door->SetOpen(false);
            break;
        }
    }
}