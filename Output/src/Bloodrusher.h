#pragma once

#include "Enemy.h"
#include "SDL2/SDL.h"

class Bloodrusher : public Enemy
{
public:

	Bloodrusher();
	virtual ~Bloodrusher();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;

public:

private:

};
