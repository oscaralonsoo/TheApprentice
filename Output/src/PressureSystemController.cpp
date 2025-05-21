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