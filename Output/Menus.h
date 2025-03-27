#pragma once

#include "Moudle.h"
#include <list>
#include <vector>

class Menus : public Module
{
public:

    Menus();

    // Destructor
    virtual ~Menus();

    // Called before render is available
    bool Awake();

    // Called before the first frame
    bool Start();

    // Called each loop iteration
    bool Update(float dt);

    // Called before quitting
    bool CleanUp();
public:

private:
};