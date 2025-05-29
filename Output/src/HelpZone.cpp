#include "HelpZone.h"
#include "Engine.h"
#include "Log.h"
#include "Textures.h"
#include "pugixml.hpp"
#include "Scene.h"

HelpZone::HelpZone() : Entity(EntityType::HELP_ZONE), texture(nullptr) {
}

HelpZone::~HelpZone() {
}

bool HelpZone::Awake() {
    return true;
}
bool HelpZone::Start() {
    pugi::xml_document artDoc;
    if (!artDoc.load_file("art.xml")) {
        LOG("Failed to load art.xml");
        return false;
    }

    // Buscar la textura correspondiente al nombre del objeto
    pugi::xml_node helpNode = artDoc.child("art").child("textures").child("UI").child("help");
    for (pugi::xml_node textureNode = helpNode.first_child(); textureNode; textureNode = textureNode.next_sibling()) {
        if (textureNode.attribute("path") && textureNode.attribute("name")) {
            std::string name = textureNode.attribute("name").as_string();
            std::string nameWithoutExt = name.substr(0, name.find_last_of('.'));
            if (nameWithoutExt == textureName) {
                std::string path = textureNode.attribute("path").as_string();
                texture = Engine::GetInstance().textures->Load((path + "/" + name).c_str());
                if (!texture) {
                    LOG("Failed to load texture: %s", (path + "/" + name).c_str());
                }
                break;
            }
        }
    }

    // Inicializar el cuerpo físico
    pbody = Engine::GetInstance().physics->CreateRectangleSensor(
        (int)position.getX() + width / 2,
        (int)position.getY() + height / 2,
        width, height,
        bodyType::STATIC,
        CATEGORY_HELPZONE,
        CATEGORY_PLAYER
    );

    pbody->ctype = ColliderType::HELPZONE;
    pbody->listener = this;

    return true;
}
bool HelpZone::Update(float dt) {
        if (fadingIn) {
            alpha += fadeSpeed * dt;
            if (alpha >= 1.0f) {
                alpha = 1.0f;
                fadingIn = false;
            }
        }
        if (fadingOut) {
            alpha -= fadeSpeed * dt;
            if (alpha <= 0.0f) {
                alpha = 0.0f;
                fadingOut = false;
            }
        }
    return true;
}
bool HelpZone::PostUpdate() {
    if (alpha > 0.0f) {
        int drawX = (int)(position.getX() - width/4);
        int drawY = (int)(position.getY() - height/1.5);

        SDL_Rect destRect = { drawX, drawY, width, height }; 
        Engine::GetInstance().render->DrawTexture(
            texture,
            static_cast<uint32_t>(destRect.x),
            static_cast<uint32_t>(destRect.y),
            nullptr,
            1.0f,
            0.0,
            0, 0,
            SDL_FLIP_NONE,
            0.65,
            alpha
        );
    }
    return true;
}

bool HelpZone::CleanUp() {
    Engine::GetInstance().physics->DeletePhysBody(pbody);
    return true;
}
void HelpZone::OnCollision(PhysBody* physA, PhysBody* physB) {
    if (physB->ctype == ColliderType::PLAYER) {
        playerInside = true;
            fadingIn = true;
            fadingOut = false;
        LOG("Player entered HelpZone: %s", textureName.c_str());
    }
}
void HelpZone::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    if (physB->ctype == ColliderType::PLAYER) {
        playerInside = false;
            fadingIn = false;
            fadingOut = true;
        LOG("Player exited HelpZone: %s", textureName.c_str());
    }
}
void HelpZone::SetWidth(int width) {
    this->width = width;
}

void HelpZone::SetHeight(int height) {
    this->height = height;
}

void HelpZone::SetTextureName(const std::string& name)
{
    textureName = name;
    LOG("HelpZone: %s", textureName.c_str());
}
