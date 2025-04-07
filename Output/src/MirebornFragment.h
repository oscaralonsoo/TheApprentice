#pragma once
#ifndef __MIREBORN_FRAGMENT_H__
#define __MIREBORN_FRAGMENT_H__

#include "Enemy.h"

class MirebornFragment : public Enemy {
public:
    MirebornFragment();
    ~MirebornFragment();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;

    void OnCollision(PhysBody* physA, PhysBody* physB) override;

private:
    // Puedes agregar variables específicas para el fragmento aquí
};

#endif // __MIREBORN_FRAGMENT_H__