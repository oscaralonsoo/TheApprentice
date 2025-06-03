#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Log.h"
#include "Physics.h"
#include "Window.h"
#include <math.h>
#include "Enemy.h"
#include "CaveDrop.h"
#include "LifePlant.h"
#include "LifePlantMax.h"
#include "Engine.h"
#include "EntityManager.h"
#include "AbilityZone.h"
#include "HiddenZone.h"
#include "Dreadspire.h"
#include "DestructibleWall.h"
#include "PushableBox.h"
#include "PressurePlate.h"
#include "PressureDoor.h"
#include "HelpZone.h"
#include "Checkpoint.h"
#include "HookAnchor.h"
#include "HokableBox.h"
#include "Geyser.h"
#include "Stalactite.h"

Map::Map() : Module(), mapLoaded(false)
{
    name = "map";
}
// Destructor
Map::~Map()
{}
// Called before render is available
bool Map::Awake()
{
    name = "map";
    LOG("Loading Map Parser");

    return true;
}

bool Map::Start() {

    return true;
}

bool Map::Update(float dt)
{
    if (mapLoaded) {
        DrawMapLayers(false); // solo capas que NO son Forward
    }
    return true;
}
bool Map::PostUpdate()
{
    if (mapLoaded) {
        DrawMapLayers(true); // solo capas que S� son Forward
    }
    return true;
}

void Map::DrawMapLayers(bool forwardOnly)
{
    for (const auto& mapLayer : mapData.layers) {
        bool isForwardLayer = (mapLayer->properties.GetProperty("Forward") != NULL &&
            mapLayer->properties.GetProperty("Forward")->value == true);

        // Filtrar seg�n par�metro
        if (isForwardLayer != forwardOnly) continue;

        if (mapLayer->properties.GetProperty("Draw") != NULL &&
            mapLayer->properties.GetProperty("Draw")->value == true) {

            Vector2D camPos = Vector2D(Engine::GetInstance().render->camera.x * -1, Engine::GetInstance().render->camera.y * -1);
            if (camPos.getX() < 0) camPos.setX(0);
            if (camPos.getY() < 0) camPos.setY(0);
            Vector2D camPosTile = WorldToMap(camPos.getX(), camPos.getY());

            int windowWidth, windowHeight;
            SDL_GetRendererOutputSize(Engine::GetInstance().render->renderer, &windowWidth, &windowHeight);

            Vector2D camOffset = {5, 5};
            Vector2D camSize = Vector2D(windowWidth, windowHeight);
            Vector2D camSizeTile = WorldToMap(camSize.getX(), camSize.getY());

            Vector2D limits = Vector2D(camPosTile.getX() + camSizeTile.getX() + camOffset.x, camPosTile.getY() + camSizeTile.getY() + camOffset.y);
            if (limits.getX() > mapData.width) limits.setX(mapData.width);
            if (limits.getY() > mapData.height) limits.setY(mapData.height);

            for (int i = camPosTile.getX(); i < limits.getX(); i++) {
                for (int j = camPosTile.getY(); j < limits.getY(); j++) {

                    uint32_t raw_gid = static_cast<uint32_t>(mapLayer->Get(i, j));
                    if (raw_gid != 0) {
                        const uint32_t FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
                        const uint32_t FLIPPED_VERTICALLY_FLAG = 0x40000000;
                        const uint32_t FLIPPED_DIAGONALLY_FLAG = 0x20000000;

                        SDL_RendererFlip flip = SDL_FLIP_NONE;
                        double angle = 0.0;

                        bool flipped_horizontally = (raw_gid & FLIPPED_HORIZONTALLY_FLAG);
                        bool flipped_vertically = (raw_gid & FLIPPED_VERTICALLY_FLAG);
                        bool flipped_diagonally = (raw_gid & FLIPPED_DIAGONALLY_FLAG);

                        if (flipped_diagonally) {
                            if (!flipped_horizontally && !flipped_vertically) {
                                angle = 270;
                                flip = SDL_FLIP_HORIZONTAL;
                            }
                            else if (flipped_horizontally && !flipped_vertically) {
                                angle = 90;
                                flip = SDL_FLIP_NONE;
                            }
                            else if (!flipped_horizontally && flipped_vertically) {
                                angle = 270;
                                flip = SDL_FLIP_NONE;
                            }
                            else if (flipped_horizontally && flipped_vertically) {
                                angle = 270;
                                flip = SDL_FLIP_VERTICAL;
                            }
                        }
                        else {
                            angle = 0;
                            if (flipped_horizontally && flipped_vertically) {
                                flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
                            }
                            else if (flipped_horizontally) {
                                flip = SDL_FLIP_HORIZONTAL;
                            }
                            else if (flipped_vertically) {
                                flip = SDL_FLIP_VERTICAL;
                            }
                        }

                        uint32_t clean_gid = raw_gid & ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG);

                        TileSet* tileSet = GetTilesetFromTileId(clean_gid);
                        if (tileSet != nullptr) {
                            SDL_Rect tileRect = tileSet->GetRect(clean_gid);
                            Vector2D mapCoord = MapToWorld(i, j);

                            int renderX = (int)mapCoord.getX();
                            int renderY = (int)mapCoord.getY();

                            uint32_t pivot = tileRect.w / 2;
                            Engine::GetInstance().render->DrawTexture(tileSet->texture, renderX, renderY, &tileRect, 1.0f, angle, pivot, pivot, flip);
                        }
                    }
                }
            }
        }
    }
}

// L09: TODO 2: Implement function to the Tileset based on a tile id
TileSet* Map::GetTilesetFromTileId(uint32_t gid) const
{
    TileSet* set = nullptr;

    for (const auto& tileset : mapData.tilesets) {
        if (gid >= tileset->firstGid && gid < (tileset->firstGid + tileset->tileCount)) {
            set = tileset;
            break;
        }
    }

    return set;
}
// Called before quitting
bool Map::CleanUp()
{
    LOG("Unloading map");

    for (PhysBody* body : Engine::GetInstance().physics->listToDelete) {
        Engine::GetInstance().physics->DeletePhysBody(body);
    }
    Engine::GetInstance().physics->listToDelete.clear();

    for (const auto& tileset : mapData.tilesets) {
        delete tileset;
    }
    mapData.tilesets.clear();

    for (const auto& layer : mapData.layers) {
        delete layer;
    }
    mapData.layers.clear();

    return true;
}

// Load new map
bool Map::Load(std::string path, std::string fileName)
{
    bool ret = false;

    // Assigns the name of the map file and the path
    mapFileName = fileName;
    mapPath = path;
    std::string mapPathName = mapPath + mapFileName;

    pugi::xml_document mapFileXML;
    pugi::xml_parse_result result = mapFileXML.load_file(mapPathName.c_str());

    if (result == NULL)
    {
        LOG("Could not load map xml file %s. pugi error: %s", mapPathName.c_str(), result.description());
        ret = false;
    }
    else {
        mapData.width = mapFileXML.child("map").attribute("width").as_int();
        mapData.height = mapFileXML.child("map").attribute("height").as_int();
        mapData.tileWidth = mapFileXML.child("map").attribute("tilewidth").as_int();
        mapData.tileHeight = mapFileXML.child("map").attribute("tileheight").as_int();

        //Iterate the Tileset
        for (pugi::xml_node tilesetNode = mapFileXML.child("map").child("tileset"); tilesetNode != NULL; tilesetNode = tilesetNode.next_sibling("tileset"))
        {
            //Load Tileset attributes
            TileSet* tileSet = new TileSet();
            tileSet->firstGid = tilesetNode.attribute("firstgid").as_int();
            tileSet->name = tilesetNode.attribute("name").as_string();
            tileSet->tileWidth = tilesetNode.attribute("tilewidth").as_int();
            tileSet->tileHeight = tilesetNode.attribute("tileheight").as_int();
            tileSet->spacing = tilesetNode.attribute("spacing").as_int();
            tileSet->margin = tilesetNode.attribute("margin").as_int();
            tileSet->tileCount = tilesetNode.attribute("tilecount").as_int();
            tileSet->columns = tilesetNode.attribute("columns").as_int();

            //Load the tileset image
            std::string imgName = tilesetNode.child("image").attribute("source").as_string();
            tileSet->texture = Engine::GetInstance().textures->Load((mapPath + imgName).c_str());

            mapData.tilesets.push_back(tileSet);
        }

        for (pugi::xml_node layerNode = mapFileXML.child("map").child("layer"); layerNode != NULL; layerNode = layerNode.next_sibling("layer")) {

            MapLayer* mapLayer = new MapLayer();
            mapLayer->id = layerNode.attribute("id").as_int();
            mapLayer->name = layerNode.attribute("name").as_string();
            mapLayer->width = layerNode.attribute("width").as_int();
            mapLayer->height = layerNode.attribute("height").as_int();

            LoadProperties(layerNode, mapLayer->properties);

            //Iterate over all the tiles and assign the values in the data array
            for (pugi::xml_node tileNode = layerNode.child("data").child("tile"); tileNode != NULL; tileNode = tileNode.next_sibling("tile")) {
                mapLayer->tiles.push_back(tileNode.attribute("gid").as_uint());
            }

            //add the layer to the map
            mapData.layers.push_back(mapLayer);
        }

        // Create colliders
        for (pugi::xml_node objectGroupNode = mapFileXML.child("map").child("objectgroup"); objectGroupNode; objectGroupNode = objectGroupNode.next_sibling("objectgroup"))
        {
            std::string objectGroupName = objectGroupNode.attribute("name").as_string();
            std::string layerName = objectGroupNode.attribute("name").as_string();

            if (objectGroupName == "Collisions") // Objects from layer Collisions
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width = objectNode.attribute("width").as_int();
                    int height = objectNode.attribute("height").as_int();

                    PhysBody* platformCollider = Engine::GetInstance().physics->CreateRectangle(x + (width / 2), y + (height / 2), width, height, STATIC);
                    platformCollider->ctype = ColliderType::PLATFORM;

                    Engine::GetInstance().physics->listToDelete.push_back(platformCollider);

                    LOG("Creating collider at x: %d, y: %d, width: %d, height: %d", x + (width / 2), y + (height / 2), width, height);
                }
            }
            else if (objectGroupName == "WallSlide") // Objects from layer Collisions
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width = objectNode.attribute("width").as_int();
                    int height = objectNode.attribute("height").as_int();

                    PhysBody* wallSlideCollider = Engine::GetInstance().physics->CreateRectangle(x + (width / 2), y + (height / 2), width, height, STATIC);
                    wallSlideCollider->ctype = ColliderType::WALL_SLIDE;

                    Engine::GetInstance().physics->listToDelete.push_back(wallSlideCollider);

                    LOG("Creating collider at x: %d, y: %d, width: %d, height: %d", x + (width / 2), y + (height / 2), width, height);
                }
            }
            else if (objectGroupName == "Spikes") // Objects from layer Collisions
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width = objectNode.attribute("width").as_int();
                    int height = objectNode.attribute("height").as_int();

                    PhysBody* spikeCollider = Engine::GetInstance().physics->CreateRectangle(x + (width / 2), y + (height / 2), width, height, STATIC);
                    spikeCollider->ctype = ColliderType::SPIKE;

                    Engine::GetInstance().physics->listToDelete.push_back(spikeCollider);

                    LOG("Creating collider at x: %d, y: %d, width: %d, height: %d", x + (width / 2), y + (height / 2), width, height);
                }
            }
            else if (objectGroupName == "Liana") // Objects from layer Collisions
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width = objectNode.attribute("width").as_int();
                    int height = objectNode.attribute("height").as_int();

                    PhysBody* lianaCollider = Engine::GetInstance().physics->CreateRectangleSensor(x + (width / 2), y + (height / 2), width, height, STATIC, CATEGORY_LIANA, CATEGORY_PLAYER);
                    lianaCollider->ctype = ColliderType::LIANA;

                    Engine::GetInstance().physics->listToDelete.push_back(lianaCollider);

                    LOG("Creating collider at x: %d, y: %d, width: %d, height: %d", x + (width / 2), y + (height / 2), width, height);
                }
            }
            else if (objectGroupName == "HookAnchors")
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width = objectNode.attribute("width").as_int();
                    int height = objectNode.attribute("height").as_int();
                    std::string texturePath = objectNode.attribute("texture").as_string();

                    pugi::xml_document tempDoc;
                    pugi::xml_node node = tempDoc.append_child("hook_anchor");
                    node.append_attribute("x") = x;
                    node.append_attribute("y") = y;
                    node.append_attribute("w") = width;
                    node.append_attribute("h") = height;
                    node.append_attribute("texture") = texturePath.c_str();

                    HookAnchor* anchor = (HookAnchor*)Engine::GetInstance().entityManager->CreateEntity(EntityType::HOOK_ANCHOR);
                    if (anchor != nullptr)
                    {
                        anchor->SetParameters(node);
                        LOG("Created HookAnchor at x: %d, y: %d", x, y);
                    }
                    else
                    {
                        LOG("ERROR: No se pudo crear HookAnchor. Verifica que esté registrado en EntityManager.");
                    }
                }
            }
            else if (objectGroupName == "Wall") // Objects from layer Collisions
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width = objectNode.attribute("width").as_int();
                    int height = objectNode.attribute("height").as_int();

                    PhysBody* wallCollider = Engine::GetInstance().physics->CreateRectangle(x + (width / 2), y + (height / 2), width, height, STATIC);
                    wallCollider->ctype = ColliderType::WALL;

                    Engine::GetInstance().physics->listToDelete.push_back(wallCollider);

                    LOG("Creating collider at x: %d, y: %d, width: %d, height: %d", x + (width / 2), y + (height / 2), width, height);
                }
            }
            else if (objectGroupName == "HelpZone")
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    std::string objectName = objectNode.attribute("name").as_string();
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width = objectNode.attribute("width").as_int();
                    int height = objectNode.attribute("height").as_int();

                    HelpZone* helpZone = (HelpZone*)Engine::GetInstance().entityManager->CreateEntity(EntityType::HELP_ZONE);
                    helpZone->position = Vector2D(x, y);
                    helpZone->SetWidth(width);
                    helpZone->SetHeight(height);
                    helpZone->SetTextureName(objectName);

                    LOG("Created HelpZone entity '%s' at x: %d, y: %d", objectName.c_str(), x, y);
                }
            }
            else if (layerName == "Doors")  // Objects from layer Doors
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode != NULL; objectNode = objectNode.next_sibling("object"))
                {
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width = objectNode.attribute("width").as_int();
                    int height = objectNode.attribute("height").as_int();

                    PhysBody* doorCollider = Engine::GetInstance().physics->CreateRectangleSensor(x + (width / 2), y + (height / 2), width, height, STATIC, CATEGORY_DOOR, CATEGORY_PLAYER);
                    doorCollider->ctype = ColliderType::DOOR;

                    // Access Properties by Name
                    for (pugi::xml_node propertyNode = objectNode.child("properties").child("property"); propertyNode != NULL; propertyNode = propertyNode.next_sibling("property"))
                    {
                        std::string propertyName = propertyNode.attribute("name").as_string();

                        if (propertyName == "TargetScene")
                        {
                            doorCollider->targetScene = propertyNode.attribute("value").as_int();
                            LOG("TargetScene: %d", doorCollider->targetScene);
                        }
                        else if (propertyName == "PlayerPosX")
                        {
                            doorCollider->playerPosX = propertyNode.attribute("value").as_float();
                            LOG("PlayerPosX: %f", doorCollider->playerPosX);
                        }
                        else if (propertyName == "PlayerPosY")
                        {
                            doorCollider->playerPosY = propertyNode.attribute("value").as_float();
                            LOG("PlayerPosY: %f", doorCollider->playerPosY);
                        }
                    }

                    Engine::GetInstance().physics->listToDelete.push_back(doorCollider);

                    LOG("Creating SceneDoor at x: %d, y: %d, width: %d, height: %d", x + (width / 2), y + (height / 2), width, height);
                }
            }
            else if (objectGroupName == "DownCamera") // Objects from layer DownCamera
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width = objectNode.attribute("width").as_int();
                    int height = objectNode.attribute("height").as_int();

                    PhysBody* downCameraCollider = Engine::GetInstance().physics->CreateRectangleSensor(
                        x + (width / 2),
                        y + (height / 2),
                        width, height,
                        STATIC,
                        CATEGORY_DOWN_CAMERA, 
                        CATEGORY_PLAYER       
                    );
                    downCameraCollider->ctype = ColliderType::DOWN_CAMERA;

                    Engine::GetInstance().physics->listToDelete.push_back(downCameraCollider);
                }
            }
            else if (objectGroupName == "Abilities") //Abilities from object layer "Abilities"
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    std::string abilityName = objectNode.attribute("name").as_string();
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();

                    int width, height;
                    GetAbilityDimensionsFromConfig(abilityName, width, height);

                    if (objectNode.attribute("width"))
                        width = objectNode.attribute("width").as_int();
                    if (objectNode.attribute("height"))
                        height = objectNode.attribute("height").as_int();

                    pugi::xml_document tempDoc;
                    pugi::xml_node abilityNode = tempDoc.append_child("abilities");

                    abilityNode.append_attribute("type") = abilityName.c_str();
                    abilityNode.append_attribute("x") = x + width / 2;
                    abilityNode.append_attribute("y") = y;
                    abilityNode.append_attribute("w") = width;
                    abilityNode.append_attribute("h") = height;

                    AbilityZone* abilityZone = nullptr;

                    if (abilityName == "Jump" || abilityName == "DoubleJump" || abilityName == "Dash" || abilityName == "Glide" || abilityName == "WallJump" || abilityName == "Hook" || abilityName == "Push")
                        abilityZone = (AbilityZone*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ABILITY_ZONE);
                    if (abilityZone != nullptr)
                    {
                        abilityZone->SetParameters(abilityNode);

                        printf("[Map] AbilityZone final size: type=%s, x=%d, y=%d, w=%d, h=%d\n",
                            abilityName.c_str(), x, y, width, height);
                    }
                }
            }
            else if (objectGroupName == "DestructibleWalls")
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width = objectNode.attribute("width").as_int();
                    int height = objectNode.attribute("height").as_int();
                    std::string texturePath = objectNode.attribute("texture").as_string();

                    pugi::xml_document tempDoc;
                    pugi::xml_node node = tempDoc.append_child("wall");
                    node.append_attribute("x") = x;
                    node.append_attribute("y") = y;
                    node.append_attribute("w") = width;
                    node.append_attribute("h") = height;
                    node.append_attribute("texture") = texturePath.c_str();

                    DestructibleWall* wall = (DestructibleWall*)Engine::GetInstance().entityManager->CreateEntity(EntityType::DESTRUCTIBLE_WALL);
                    wall->SetParameters(node);
                }
            }
            else if (objectGroupName == "HookableBoxes")
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width = objectNode.attribute("width").as_int();
                    int height = objectNode.attribute("height").as_int();
                    std::string texturePath = objectNode.attribute("texture").as_string();

                    pugi::xml_document tempDoc;
                    pugi::xml_node node = tempDoc.append_child("hookable_box");
                    node.append_attribute("x") = x;
                    node.append_attribute("y") = y;
                    node.append_attribute("w") = width;
                    node.append_attribute("h") = height;
                    node.append_attribute("texture") = texturePath.c_str();

                    HookableBox* box = (HookableBox*)Engine::GetInstance().entityManager->CreateEntity(EntityType::HOOKABLE_BOX);
                    box->SetParameters(node);
                }
            }
            else if (objectGroupName == "Props")
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    std::string objectName = objectNode.attribute("name").as_string();
                    if (objectName == "CaveDrop")
                    {
                        int x = objectNode.attribute("x").as_int();
                        int y = objectNode.attribute("y").as_int();

                        CaveDrop* caveDrop = (CaveDrop*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CAVE_DROP);
                        caveDrop->position = Vector2D(x, y);

                        LOG("Created CaveDrop at x: %d, y: %d", x, y);
                    }
                    else if (objectName == "Stalactite") {
                        int x = objectNode.attribute("x").as_int();
                        int y = objectNode.attribute("y").as_int();
                        int stalactiteVariant = objectNode.child("properties").child("property").attribute("value").as_int();
                        Stalactite* stalactite = (Stalactite*)Engine::GetInstance().entityManager->CreateEntity(EntityType::STALACTITE);
                        stalactite->position = Vector2D(x, y);
                        stalactite->variant = stalactiteVariant;

                        LOG("Created Stalactite at x: %d, y: %d", x, y);
                    }
                    else if (objectName == "LifePlant") {
                        int x = objectNode.attribute("x").as_int();
                        int y = objectNode.attribute("y").as_int();

                        LifePlant* lifePlant = (LifePlant*)Engine::GetInstance().entityManager->CreateEntity(EntityType::LIFE_PLANT);
                        lifePlant->position = Vector2D(x, y);

                        LOG("Created LifePlant at x: %d, y: %d", x, y);
                    }
                    else if (objectName == "LifePlantMax") {
                        int x = objectNode.attribute("x").as_int();
                        int y = objectNode.attribute("y").as_int();

                        LifePlantMax* lifePlantMax = (LifePlantMax*)Engine::GetInstance().entityManager->CreateEntity(EntityType::LIFE_PLANT_MAX);
                        lifePlantMax->position = Vector2D(x, y);

                        LOG("Created LifePlant at x: %d, y: %d", x, y);
                    }
                    else if (objectName == "Checkpoint")
                    {
                        int x = objectNode.attribute("x").as_int();
                        int y = objectNode.attribute("y").as_int();
                        int width = objectNode.attribute("width").as_int();
                        int height = objectNode.attribute("height").as_int();

                        Checkpoint* checkpoint = (Checkpoint*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CHECKPOINT);
                        checkpoint->position = Vector2D(x, y);
                        checkpoint->height = height;
                        checkpoint->width = width;

                        LOG("Created Checkpoint at x: %d, y: %d", x, y);
                    }
                    else if (objectName == "Geyser")
                    {
                        int x = objectNode.attribute("x").as_int();
                        int y = objectNode.attribute("y").as_int();
                        int width = objectNode.attribute("width").as_int();
                        int height = objectNode.attribute("height").as_int();

                        Geyser* geyser = (Geyser*)Engine::GetInstance().entityManager->CreateEntity(EntityType::GEYSER);
                        geyser->position = Vector2D(x, y);
                        geyser->height = height;
                        geyser->width = width;

                        LOG("Created Geyser at x: %d, y: %d", x, y);
                    }
                    else if (objectName == "Box")
                    {
                        int x = objectNode.attribute("x").as_int();
                        int y = objectNode.attribute("y").as_int();
                        int width = objectNode.attribute("width").as_int();
                        int height = objectNode.attribute("height").as_int();

                        pugi::xml_document tempDoc;
                        pugi::xml_node node = tempDoc.append_child("box");
                        node.append_attribute("x") = x;
                        node.append_attribute("y") = y;
                        node.append_attribute("w") = width;
                        node.append_attribute("h") = height;

                        PushableBox* box = (PushableBox*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PUSHABLE_BOX);
                        box->SetParameters(node);
                    }
                }
            }
            else if (objectGroupName == "Zones")
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    std::string objectName = objectNode.attribute("name").as_string();
                    if (objectName == "HiddenZone")
                    {
                        int x = objectNode.attribute("x").as_int();
                        int y = objectNode.attribute("y").as_int();
                        int w = objectNode.attribute("width").as_int();
                        int h = objectNode.attribute("height").as_int();

                        HiddenZone* hiddenZone = (HiddenZone*)Engine::GetInstance().entityManager->CreateEntity(EntityType::HIDDEN_ZONE);
                        hiddenZone->position = Vector2D(x, y);
                        hiddenZone->SetWidth(w);
                        hiddenZone->SetHeight(h);

                        LOG("Created Hidden Zone at x: %d, y: %d", x, y);
                    }
                }
            }
            else if (objectGroupName == "Enemies")
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    std::string enemyName = objectNode.attribute("name").as_string();
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width, height;
                    GetEnemyDimensionsFromConfig(enemyName, width, height);

                    pugi::xml_document tempDoc;
                    pugi::xml_node enemyNode = tempDoc.append_child("enemy");

                    enemyNode.append_attribute("type") = enemyName.c_str();
                    enemyNode.append_attribute("x") = x;
                    enemyNode.append_attribute("y") = y;
                    enemyNode.append_attribute("w") = width;
                    enemyNode.append_attribute("h") = height;


                    Enemy* enemy = nullptr;

                    if (enemyName == "Bloodrusher")
                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BLOODRUSHER);
                    else if (enemyName == "Hypnoviper")
                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::HYPNOVIPER);
                    else if (enemyName == "Creebler") {
                        for (pugi::xml_node propertyNode = objectNode.child("properties").child("property"); propertyNode != NULL; propertyNode = propertyNode.next_sibling("property")) {
                            std::string propertyName = propertyNode.attribute("name").as_string();

                            if (propertyName == "navigationId")
                                enemyNode.append_attribute("navigationId") = propertyNode.attribute("value").as_int();

                            else if (propertyName == "navigationLayer")
                                enemyNode.append_attribute("navigationLayer") = propertyNode.attribute("value").as_string();
                        }

                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CREEBLER);
                    }
                    else if (enemyName == "Scurver") {
                        enemyNode.append_attribute("gravity") = true;
                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::SCURVER);
                    }
                    else if (enemyName == "Shyver") {
                        enemyNode.append_attribute("gravity") = false;
                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::SHYVER);
                    }
                    else if (enemyName == "Nullwarden")
                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::NULLWARDEN);
                    else if (enemyName == "Thumpod")
                    {
                        enemyNode.append_attribute("gravity") = true;
                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::THUMPOD);
                    }
                    else if (enemyName == "Mireborn") {
                        enemyNode.append_attribute("gravity") = true;
                        enemyNode.append_attribute("tier") = "Alpha";
                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::MIREBORN);
                    }
                    else if (enemyName == "Broodheart") {
                        enemyNode.append_attribute("gravity") = false;
                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BROODHEART);
                    }
                    else if (enemyName == "Brood") {
                        enemyNode.append_attribute("gravity") = true;
                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BROOD);
                    }
                    else if (enemyName == "Noctilume") {
                        enemyNode.append_attribute("gravity") = true;
                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::NOCTILUME);
                    }
                    else if (enemyName == "Dreadspire") {
                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::DREADSPIRE);
                        for (pugi::xml_node propertyNode = objectNode.child("properties").child("property"); propertyNode != NULL; propertyNode = propertyNode.next_sibling("property"))
                        {
                            std::string propertyName = propertyNode.attribute("name").as_string();

                            if (propertyName == "upsiteDown")
                                enemy->upsiteDown = propertyNode.attribute("value").as_bool();
                        }
                    }
                    else if (enemyName == "DungBeetle") {
                        enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::DUNGBEETLE);
                    }

                    if (enemy != nullptr)
                    {
                        enemy->SetParameters(enemyNode);
                        LOG("Created enemy '%s' at x: %d, y: %d", enemyName.c_str(), x, y);
                    }
                }
            }
            else if (objectGroupName == "NPC")
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    std::string npcName = objectNode.attribute("name").as_string();
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int w = objectNode.attribute("width").as_int();
                    int h = objectNode.attribute("height").as_int();
                    int dialogueId = objectNode.child("properties").child("property").attribute("value").as_int();

                    pugi::xml_document tempDoc;
                    pugi::xml_node npcNode = tempDoc.append_child("enemy");

                    npcNode.append_attribute("type") = npcName.c_str();
                    npcNode.append_attribute("x") = x;
                    npcNode.append_attribute("y") = y;
                    npcNode.append_attribute("w") = w;
                    npcNode.append_attribute("h") = h;
                    npcNode.append_attribute("dialogueId") = dialogueId;
                    npcNode.append_attribute("gravity") = true;

                    Enemy* npc = nullptr;

                    if (npcName == "Castor")
                        npc = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CASTOR);
                    else if (npcName == "Liebre")
                        npc = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::LIEBRE);
                    else if (npcName == "Perdiz")
                        npc = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PERDIZ);
                    else if (npcName == "Pangolin")
                        npc = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PANGOLIN);
                    else if (npcName == "Ardilla")
                        npc = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ARDILLA);
                    else if (npcName == "CaracolMail")
                        npc = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CARACOL_MAIL);
                    else if (npcName == "Carta")
                        npc = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CARTA);
                    else if (npcName == "ArdillaLiana")
                        npc = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ARDILLA_LIANA);
                    else if (npcName == "Caracol")
                        npc = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CARACOL);
                    else if (npcName == "BichoPalo")
                        npc = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BICHOPALO);

                    if (npc != nullptr)
                    {
                        npc->SetParameters(npcNode);
                        LOG("Created NPC '%s' at x: %d, y: %d", npcName.c_str(), x, y);
                    }
                }
            }
            else if (objectGroupName == "PressureSystem")
            {
                std::vector<PressurePlate*> plates;
                std::vector<PressureDoor*> doors;

                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    std::string objectName = objectNode.attribute("name").as_string();

                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int w = objectNode.attribute("width").as_int();
                    int h = objectNode.attribute("height").as_int();

                    int id = 0;
                    bool isInvisible = false; 
                    bool isOpen = false;

                    // Accede a las propiedades del objeto
                    for (pugi::xml_node propertyNode = objectNode.child("properties").child("property"); propertyNode; propertyNode = propertyNode.next_sibling("property"))
                    {
                        std::string propertyName = propertyNode.attribute("name").as_string();
                        if (propertyName == "groupId")
                        {
                            id = propertyNode.attribute("value").as_int();
                        }
                        else if (propertyName == "isInvisible")
                        {
                            isInvisible = propertyNode.attribute("value").as_bool();
                        }
                        else if (propertyName == "isOpen")
                        {
                            isOpen = propertyNode.attribute("value").as_bool();
                        }
                    }

                    if (objectName == "Plate")
                    {
                        PressurePlate* plate = (PressurePlate*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PRESSURE_PLATE);
                        plate->position = Vector2D(x, y);
                        plate->isInvisible = isInvisible;
                        plate->id = id;
                        plates.push_back(plate);

                        LOG("Created Plate at x: %d, y: %d", x, y);
                    }
                    else if (objectName == "Door")
                    {
                        PressureDoor* door = (PressureDoor*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PRESSURE_DOOR);
                        door->position = Vector2D(x, y);
                        door->width = w;
                        door->height = h;
                        door->id = id;
                        door->shouldBeOpen = isOpen;
                        doors.push_back(door);

                        LOG("Created Door at x: %d, y: %d", x, y);
                    }
                }
                Engine::GetInstance().pressureSystem->plates = plates;
                Engine::GetInstance().pressureSystem->doors = doors;
            }
        }
        ret = true;


        if (ret == true)
        {
            LOG("Successfully parsed map XML file :%s", fileName.c_str());
            LOG("width : %d height : %d", mapData.width, mapData.height);
            LOG("tile_width : %d tile_height : %d", mapData.tileWidth, mapData.tileHeight);

            LOG("Tilesets----");

            for (const auto& tileset : mapData.tilesets) {
                LOG("name : %s firstgid : %d", tileset->name.c_str(), tileset->firstGid);
                LOG("tile width : %d tile height : %d", tileset->tileWidth, tileset->tileHeight);
                LOG("spacing : %d margin : %d", tileset->spacing, tileset->margin);
            }

            LOG("Layers----");

            for (const auto& layer : mapData.layers) {
                LOG("id : %d name : %s", layer->id, layer->name.c_str());
                LOG("Layer width : %d Layer height : %d", layer->width, layer->height);
            }
        }
        else {
            LOG("Error while parsing map file: %s", mapPathName.c_str());
        }

        if (mapFileXML) mapFileXML.reset();

    }

    mapLoaded = ret;
    return ret;
}

Vector2D Map::MapToWorld(int x, int y) const
{
    Vector2D ret;

    ret.setX(x * mapData.tileWidth);
    ret.setY(y * mapData.tileHeight);

    return ret;
}

Vector2D Map::WorldToMap(int x, int y) {

    Vector2D ret(0, 0);

    ret.setX(x / mapData.tileWidth);
    ret.setY(y / mapData.tileHeight);

    return ret;
}

bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
    bool ret = false;

    for (pugi::xml_node propertieNode = node.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
    {
        Properties::Property* p = new Properties::Property();
        p->name = propertieNode.attribute("name").as_string();
        p->value = propertieNode.attribute("value").as_bool();
        p->number = propertieNode.attribute("number").as_int();

        properties.propertyList.push_back(p);
    }

    return ret;
}

MapLayer* Map::GetNavigationLayer() {
    for (const auto& layer : mapData.layers) {
        if (layer->properties.GetProperty("Navigation") != NULL &&
            layer->properties.GetProperty("Navigation")->value) {
            return layer;
        }
    }

    return nullptr;
}

MapLayer* Map::GetNavigationLayerByName(const std::string& name) {
    for (const auto& layer : mapData.layers) {
        if (layer->name == name &&
            layer->properties.GetProperty("Navigation") != nullptr &&
            layer->properties.GetProperty("Navigation")->value) {
            return layer;
        }
    }
    return nullptr;
}

void Map::SetNavigationTileRegion(int x, int y, int w, int h, uint32_t newTileId) {
    for (const auto& layer : mapData.layers) {
        if (layer->properties.GetProperty("Navigation") != nullptr &&
            layer->properties.GetProperty("Navigation")->value) {

            for (int j = 0; j < h; ++j) {
                for (int i = 0; i < w; ++i) {
                    int tileX = x + i;
                    int tileY = y + j;

                    if (tileX < 0 || tileY < 0 || tileX >= layer->width || tileY >= layer->height)
                        continue;

                    size_t index = tileY * layer->width + tileX;
                    layer->tiles[index] = newTileId;
                }
            }

            break;
        }
    }
}

Properties::Property* Properties::GetProperty(const char* name)
{
    for (const auto& property : propertyList) {
        if (property->name == name) {
            return property;
        }
    }

    return nullptr;
}

void Map::GetEnemyDimensionsFromConfig(const std::string& enemyName, int& width, int& height)
{
    pugi::xml_document configDoc;
    configDoc.load_file("config.xml");

    pugi::xml_node enemyNode = configDoc.child("config").child("scene").child("animations").child("enemies").child("enemy");
    while (enemyNode) {
        if (enemyNode.attribute("type").as_string() == enemyName) {
            width = enemyNode.attribute("w").as_int();
            height = enemyNode.attribute("h").as_int();
            return;
        }
        enemyNode = enemyNode.next_sibling("enemy");
    }
}

void Map::GetAbilityDimensionsFromConfig(const std::string& abilityName, int& width, int& height)
{
    pugi::xml_document configDoc;
    configDoc.load_file("config.xml");

    pugi::xml_node abilityNode = configDoc.child("config").child("scene").child("animations").child("abilities").child("ability");
    while (abilityNode) {
        if (abilityNode.attribute("type").as_string() == abilityName) {
            width = abilityNode.attribute("w").as_int();
            height = abilityNode.attribute("h").as_int();
            return;
        }
        abilityNode = abilityNode.next_sibling("ability");
    }

    width = 50;
    height = 50;
}