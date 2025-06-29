
#include "Entity.h"
#include "Physics.h"
#include "Textures.h"

class HelpZone : public Entity {
public:
    HelpZone();
    ~HelpZone();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate() override;
    bool CleanUp() override;

    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;

    void SetWidth(int width);
    void SetHeight(int height);
    void SetTextureName(const std::string& name);

public:
    bool playerInside =false;
private:
    PhysBody* pbody;
    std::string textureName; 
    SDL_Texture* texture; 

    //Fade In/Out
    bool fadingIn = false;
    bool fadingOut = false;
    float alpha = 0.0f;
    float fadeSpeed = 0.005f; 
};