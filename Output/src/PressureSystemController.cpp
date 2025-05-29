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
    std::unordered_map<int, int> totalInvisiblePlates;
    std::unordered_map<int, int> activeInvisiblePlates;

    for (auto* plate : plates)
    {
        totalPlates[plate->id]++;
        if (plate->IsActive())
        {
            activePlates[plate->id]++;
        }

        if (plate->isInvisible)
        {
            totalInvisiblePlates[plate->id]++;
            if (plate->IsActive())
            {
                activeInvisiblePlates[plate->id]++;
            }
        }
    }

    for (auto* door : doors)
    {
        if (totalInvisiblePlates[door->id] > 0)
        {
            bool allInvisiblesActive = (activeInvisiblePlates[door->id] == totalInvisiblePlates[door->id]);

            if (allInvisiblesActive && !door->triggeredOnce)
            {
                door->SetOpen(true);
                door->triggeredOnce = true;
                door->shouldBeOpen = !door->shouldBeOpen;
            }
            continue;
        }
        bool shouldOpen = (totalPlates[door->id] > 0 && activePlates[door->id] == totalPlates[door->id]);
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
            door->state = PressureDoorState::DISABLE;
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