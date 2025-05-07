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

#define CATEGORY_PLAYER           0x0001 
#define CATEGORY_ENEMY            0x0002  
#define CATEGORY_PLAYER_DAMAGE    0x0004  
#define CATEGORY_PLATFORM         0x0008  
#define CATEGORY_WALL             0x0010  
#define CATEGORY_SPIKE            0x0020  
#define CATEGORY_ABILITY_ZONE     0x0040  
#define CATEGORY_HIDDEN_ZONE      0x0080
#define CATEGORY_DUST_PARTICLE    0x0100
#define CATEGORY_SAVEGAME         0x0200
#define CATEGORY_DOWN_CAMERA      0x0400
#define CATEGORY_ATTACK           0x0800
#define CATEGORY_CAVE_DROP        0x1000
#define CATEGORY_NPC              0x2000
#define CATEGORY_DOOR             0x4000
#define CATEGORY_LIFE_PLANT       0x8000
#define CATEGORY_GEYSER           0x0080
#define CATEGORY_HELPZONE         0x0080

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
	NPC,
	HELPZONE,
	ENEMY,
	PLATFORM, 
	CHECKPOINT,
	DOOR,
	ABILITY_ZONE,
	HIDDEN_ZONE,
	DESTRUCTIBLE_WALL,
	PUSHABLE_PLATFORM,
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
	PhysBody* CreateCircleSensor(int x, int y, int radius, bodyType type);
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