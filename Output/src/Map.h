#pragma once

#include "Module.h"
#include <list>
#include <vector>

// L09: TODO 5: Add attributes to the property structure
struct Properties
{
    struct Property
    {
        std::string name;
        bool value;
        int number;
    };

    std::list<Property*> propertyList;

    ~Properties()
    {
        for (const auto& property : propertyList)
        {
            delete property;
        }

        propertyList.clear();
    }

    // L09: DONE 7: Method to ask for the value of a custom property
    Property* GetProperty(const char* name);

};

struct MapLayer
{
    // L07: TODO 1: Add the info to the MapLayer Struct
    int id;
    std::string name;
    int width;
    int height;
    std::vector<uint32_t> tiles;
    Properties properties;

    // L07: TODO 6: Short function to get the gid value of i,j
    uint32_t Get(uint32_t i, uint32_t j) const
    {
        if (i >= width || j >= height)
            return 0;
        return tiles[(j * width) + i];
    }
};

// L06: TODO 2: Create a struct to hold information for a TileSet
// Ignore Terrain Types and Tile Types for now, but we want the image!

struct TileSet
{
    int firstGid;
    std::string name;
    int tileWidth;
    int tileHeight;
    int spacing;
    int margin;
    int tileCount;
    int columns;
    SDL_Texture* texture;

    // L07: TODO 7: Implement the method that receives the gid and returns a Rect
    SDL_Rect GetRect(uint32_t gid) {
        SDL_Rect rect = { 0 };

        uint32_t relativeIndex = gid - firstGid;
        rect.w = tileWidth;
        rect.h = tileHeight;
        rect.x = margin + (tileWidth + spacing) * (relativeIndex % columns);
        rect.y = margin + (tileHeight + spacing) * (relativeIndex / columns);

        return rect;
    }

};

// L06: TODO 1: Create a struct needed to hold the information to Map node
struct MapData
{
    int width;
    int height;
    int tileWidth;
    int tileHeight;
    std::list<TileSet*> tilesets;

    // L07: TODO 2: Add the info to the MapLayer Struct
    std::list<MapLayer*> layers;
};

class Map : public Module
{
public:

    Map();

    // Destructor
    virtual ~Map();

    // Called before render is available
    bool Awake();

    // Called before the first frame
    bool Start();

    // Called each loop iteration
    bool Update(float dt);

    // Called each loop after iteration
    bool PostUpdate();

    // Called before quitting
    bool CleanUp();

    // Load new map
    bool Load(std::string path, std::string mapFileName);

    void DrawMapLayers(bool forwardOnly);

    // L07: TODO 8: Create a method that translates x,y coordinates from map positions to world positions
    Vector2D MapToWorld(int x, int y) const;

    // L10: TODO 5: Add method WorldToMap to obtain  map coordinates from screen coordinates 
    Vector2D WorldToMap(int x, int y);

    // L09: TODO 2: Implement function to the Tileset based on a tile id
    TileSet* GetTilesetFromTileId(uint32_t gid) const;

    void GetEnemyDimensionsFromConfig(const std::string& enemyName, int& width, int& height);
    void GetAbilityDimensionsFromConfig(const std::string& enemyName, int& width, int& height);
    // L09: TODO 6: Load a group of properties 
    bool LoadProperties(pugi::xml_node& node, Properties& properties);
    bool IsPlatformTile(int tileX, int tileY) const;
    int GetWidth() {
        return mapData.width;
    }

    int GetHeight() {
        return mapData.height;
    }

    int GetTileWidth() {
        return mapData.tileWidth;
    }

    int GetTileHeight() {
        return mapData.tileHeight;
    }
    int GetHeightPixels() const { return mapData.height * mapData.tileHeight; }
    int GetMapWidth() const { return mapData.width * mapData.tileWidth; }
    int GetMapHeight() const { return mapData.height * mapData.tileHeight; }

    MapLayer* GetNavigationLayer();
    MapLayer* GetNavigationLayerByName(const std::string& name);
    void SetNavigationTileRegion(int x, int y, int w, int h, uint32_t newTileId);

public:
    std::string mapFileName;
    std::string mapPath;

private:
    bool mapLoaded;
    // L06: DONE 1: Declare a variable data of the struct MapData
    MapData mapData;
};