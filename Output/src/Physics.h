#pragma once
#include "Module.h"
#include "Entity.h"
#include "box2d/box2d.h"
#include <vector>
#include <list>

#define GRAVITY_X 0.0f
#define GRAVITY_Y -15.0f

#define PIXELS_PER_METER 50.0f // if touched change METER_PER_PIXEL too
#define METER_PER_PIXEL 0.02f // this is 1 / PIXELS_PER_METER !

#define METERS_TO_PIXELS(m) ((int) floor(PIXELS_PER_METER * m))
#define PIXEL_TO_METERS(p)  ((float) METER_PER_PIXEL * p)

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f

#define CATEGORY_PLAYER           1 
#define CATEGORY_ENEMY            2  
#define CATEGORY_PLAYER_DAMAGE    3  
#define CATEGORY_PLATFORM         4  
#define CATEGORY_WALL             5  
#define CATEGORY_SPIKE            6  
#define CATEGORY_ABILITY_ZONE     7  
#define CATEGORY_HIDDEN_ZONE      8
#define CATEGORY_DUST_PARTICLE    9
#define CATEGORY_SAVEGAME         10
#define CATEGORY_DOWN_CAMERA      11
#define CATEGORY_ATTACK           12
#define CATEGORY_CAVE_DROP        13
#define CATEGORY_NPC              14
#define CATEGORY_DOOR             15
#define CATEGORY_LIFE_PLANT       16
#define CATEGORY_GEYSER           17
#define CATEGORY_HELPZONE         18
#define CATEGORY_PRESSURE_PLATE   19
#define CATEGORY_BOX			  20
#define CATEGORY_HOOK_SENSOR	  21

// types of bodies
enum bodyType {
	DYNAMIC,
	STATIC,
	KINEMATIC
};

enum class ColliderType {
	PLAYER, 
	PLAYER_DAMAGE,
	LIFE_PLANT,
	SPIKE,
	WALL_SLIDE,
	WALL,
	DOWN_CAMERA,
	ATTACK,
	GEYSER,
	CAVE_DROP,
	PRESSURE_PLATE,
	NPC,
	HELPZONE,
	ENEMY,
	PLATFORM, 
	CHECKPOINT,
	DOOR,
	ABILITY_ZONE,
	HIDDEN_ZONE,
	DESTRUCTIBLE_WALL,
	HOOK_ANCHOR,
	HOOK_SENSOR,
	BOX,
	UNKNOWN
};

// Small class to return to other modules to track position and rotation of physics bodies
class PhysBody
{
public:
	PhysBody() : listener(NULL), body(NULL), ctype(ColliderType::UNKNOWN)
	{}

	~PhysBody() {}

	void GetPosition(int& x, int& y) const;
	float GetRotation() const;
	bool Contains(int x, int y) const;
	int RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const;

public:
	int width = 0;
	int height = 0;
	// Door Values
	int targetScene = 0;
	float playerPosX = 0.0f;
	float playerPosY = 0.0f;
	b2Body* body;
	std::string objectName;
	Entity* listener;
	ColliderType ctype;
};

// Module --------------------------------------
class Physics : public Module, public b2ContactListener // TODO
{
public:

	// Constructors & Destructors
	Physics();
	~Physics();

	// Main module steps
	bool Start();
	bool PreUpdate();
	bool PostUpdate();
	bool CleanUp();

	// Create basic physics objects
	PhysBody* CreateRectangle(int x, int y, int width, int height, bodyType type, float offsetX = 0, float offsetY = 0, uint16 categoryBits = 0xFFFF, uint16 maskBits = 0xFFFF);
	PhysBody* CreateCircle(int x, int y, int radious, bodyType type);
	PhysBody* CreateRectangleSensor(int x, int y, int width, int height, bodyType type, uint16 categoryBits, uint16 maskBits);
	PhysBody* CreatePolygon(int x, int y, const std::vector<b2Vec2>& vertices, bodyType type);
	PhysBody* CreateCircleSensor(int x, int y, int radius, bodyType type, uint16 categoryBits, uint16 maskBits);
	PhysBody* CreateChain(int x, int y, int* points, int size, bodyType type);
	
	// b2ContactListener ---
	void BeginContact(b2Contact* contact);
	void EndContact(b2Contact* contact);

	void DeletePhysBody(PhysBody* physBody);
	bool IsPendingToDelete(PhysBody* physBody);

	// List of physics bodies

	std::list<PhysBody*> bodiesToDelete;

	std::list<PhysBody*> listToDelete;
private:
	std::vector<int> forces;
	// Debug mode
	bool debug = false;

	// Box2D World
	b2World* world;

};