#pragma once

class Player;

class GodModeSystem {
public:
    void Init(Player* player);
    void Update(float dt);

    void Toggle();
    bool IsEnabled() const;

private:
    Player* player = nullptr;
    bool godModeEnabled = false;
};
