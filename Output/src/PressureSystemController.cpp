#include "PressureSystemController.h"
#include <unordered_map>
#include "LOG.h"

PressureSystemController::PressureSystemController()
{
}

PressureSystemController::~PressureSystemController() {}

void PressureSystemController::UpdateSystem()
{
    std::unordered_map<int, bool> systemActive;

    for (auto* plate : plates)
    {
        if (systemActive.find(plate->id) == systemActive.end())
        {
            systemActive[plate->id] = true;
        }

        if (!plate->IsActive())
        {
            systemActive[plate->id] = false;
        }
    }

    for (auto* door : doors)
    {
       door->SetOpen(systemActive[door->id]);
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
            door->SetOpen(true);
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