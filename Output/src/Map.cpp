#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Log.h"
#include "Physics.h"
#include "Window.h"
#include <math.h>
#include "Enemy.h"
#include "Engine.h"
#include "EntityManager.h"

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

    bool ret = true;

    if (mapLoaded) {

        // L07 TODO 5: Prepare the loop to draw all tiles in a layer + DrawTexture()
        // iterate all tiles in a layer
        for (const auto& mapLayer : mapData.layers) {
            if (mapLayer->properties.GetProperty("Draw") != NULL &&
                mapLayer->properties.GetProperty("Draw")->value == true) {

                for (int i = 0; i < mapData.width; i++) {
                    for (int j = 0; j < mapData.height; j++) {

                        int gid = mapLayer->Get(i, j);
                        if (gid != 0) {
                            TileSet* tileSet = GetTilesetFromTileId(gid);
                            if (tileSet != nullptr) {
                                SDL_Rect tileRect = tileSet->GetRect(gid);

                                // Convertir coordenadas del mapa a coordenadas de pantalla
                                Vector2D mapCoord = MapToWorld(i, j);

                                // Aplicar el efecto parallax en base a la c mara
                                int renderX = (int)(mapCoord.getX() - (Engine::GetInstance().render->camera.x * mapLayer->parallaxX));
                                int renderY = (int)(mapCoord.getY() - (Engine::GetInstance().render->camera.y * mapLayer->parallaxY));

                                Engine::GetInstance().render->DrawTexture(tileSet->texture, renderX, renderY, &tileRect);
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

// L09: TODO 2: Implement function to the Tileset based on a tile id
TileSet* Map::GetTilesetFromTileId(int gid) const
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

        // L06: TODO 3: Implement LoadMap to load the map properties
        // retrieve the paremeters of the <map> node and store the into the mapData struct
        mapData.width = mapFileXML.child("map").attribute("width").as_int();
        mapData.height = mapFileXML.child("map").attribute("height").as_int();
        mapData.tileWidth = mapFileXML.child("map").attribute("tilewidth").as_int();
        mapData.tileHeight = mapFileXML.child("map").attribute("tileheight").as_int();

        // L06: TODO 4: Implement the LoadTileSet function to load the tileset properties

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

        // L07: TODO 3: Iterate all layers in the TMX and load each of them
        for (pugi::xml_node layerNode = mapFileXML.child("map").child("layer"); layerNode != NULL; layerNode = layerNode.next_sibling("layer")) {

            // L07: TODO 4: Implement the load of a single layer 
            //Load the attributes and saved in a new MapLayer
            MapLayer* mapLayer = new MapLayer();
            mapLayer->id = layerNode.attribute("id").as_int();
            mapLayer->name = layerNode.attribute("name").as_string();
            mapLayer->width = layerNode.attribute("width").as_int();
            mapLayer->height = layerNode.attribute("height").as_int();

            //L09: TODO 6 Call Load Layer Properties
            LoadProperties(layerNode, mapLayer->properties);

            //Iterate over all the tiles and assign the values in the data array
            for (pugi::xml_node tileNode = layerNode.child("data").child("tile"); tileNode != NULL; tileNode = tileNode.next_sibling("tile")) {
                mapLayer->tiles.push_back(tileNode.attribute("gid").as_int());
            }

            // Dentro del bucle que itera sobre cada capa en Load()
            if (layerNode.attribute("parallaxx"))
                mapLayer->parallaxX = layerNode.attribute("parallaxx").as_float();
            else
                mapLayer->parallaxX = 1.0f; // Valor por defecto

            if (layerNode.attribute("parallaxy"))
                mapLayer->parallaxY = layerNode.attribute("parallaxy").as_float();
            else
                mapLayer->parallaxY = 1.0f; // Valor por defecto

            //add the layer to the map
            mapData.layers.push_back(mapLayer);
        }

        // L08 TODO 3: Create colliders
        // L08 TODO 7: Assign collider type
        // Later you can create a function here to load and create the colliders from the map

        // L08 TODO 3: Create colliders
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
            else if (objectGroupName == "SaveGame") // Objects from layer SaveGame
            {
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
                {
                    int x = objectNode.attribute("x").as_int();
                    int y = objectNode.attribute("y").as_int();
                    int width = objectNode.attribute("width").as_int();
                    int height = objectNode.attribute("height").as_int();

                    PhysBody* saveGameCollider = Engine::GetInstance().physics->CreateRectangle(x + (width / 2), y + (height / 2), width, height, STATIC);
                    saveGameCollider->ctype = ColliderType::SAVEGAME;

                    Engine::GetInstance().physics->listToDelete.push_back(saveGameCollider);

                    LOG("Creating collider at x: %d, y: %d, width: %d, height: %d", x + (width / 2), y + (height / 2), width, height);
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

                    // Create Door type Collider
                    PhysBody* doorCollider = Engine::GetInstance().physics->CreateRectangle(x + (width / 2), y + (height / 2), width, height, STATIC);
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

                    LOG("Creating Door at x: %d, y: %d, width: %d, height: %d", x + (width / 2), y + (height / 2), width, height);
                }
            }
        }
        for (const auto& mapLayer : mapData.layers) {
            if (mapLayer->name == "Enemies") {
                // Load XML config file
                pugi::xml_document loadFile;
                pugi::xml_parse_result result = loadFile.load_file("config.xml");

                // Get node save data -> enemies
                pugi::xml_node saveData = loadFile.child("config").child("scene").child("save_data");
                pugi::xml_node enemiesNode = saveData.child("enemies");

                // Remove all children
                enemiesNode.remove_children();

                for (int i = 0; i < mapData.width; i++) {
                    for (int j = 0; j < mapData.height; j++) {
                        int gid = mapLayer->Get(i, j);
                        if (gid == 105) {
                            // Convertir coordenadas del mapa a coordenadas del mundo
                            Vector2D mapCoord = MapToWorld(i, j);

                            // Crear un nuevo nodo <enemy>
                            pugi::xml_node enemyNode = enemiesNode.append_child("enemy");
                            enemyNode.append_attribute("type") = "Bloodrusher";
                            enemyNode.append_attribute("x") = mapCoord.x;
                            enemyNode.append_attribute("y") = mapCoord.y;
                            enemyNode.append_attribute("w") = 32;
                            enemyNode.append_attribute("h") = 32;
                        }
                    }
                }

                // Guardar los cambios en el archivo
                loadFile.save_file("config.xml");
                Engine::GetInstance().UpdateConfig();
            }
        }
        ret = true;

        // L06: TODO 5: LOG all the data loaded iterate all tilesetsand LOG everything
        if (ret == true)
        {
            LOG("Successfully parsed map XML file :%s", fileName.c_str());
            LOG("width : %d height : %d", mapData.width, mapData.height);
            LOG("tile_width : %d tile_height : %d", mapData.tileWidth, mapData.tileHeight);

            LOG("Tilesets----");

            //iterate the tilesets
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

// L07: TODO 8: Create a method that translates x,y coordinates from map positions to world positions
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
// L09: TODO 6: Load a group of properties from a node and fill a list with it
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

// L09: TODO 7: Implement a method to get the value of a custom property
Properties::Property* Properties::GetProperty(const char* name)
{
    for (const auto& property : propertyList) {
        if (property->name == name) {
            return property;
        }
    }

    return nullptr;
}

