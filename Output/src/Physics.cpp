#include "Physics.h"
#include "Input.h"
#include "Engine.h"
#include "Log.h"
#include "math.h"
#include "SDL2/SDL_keycode.h"
#include "Log.h"
#include "Render.h"
#include "Player.h"
#include "Window.h"
#include "box2D/box2d.h"
#include "Menus.h"

Physics::Physics() : Module()
{
	// Initialise all the internal class variables, at least to NULL pointer
	world = NULL;
}

// Destructor
Physics::~Physics()
{
	// You should do some memory cleaning here, if required
}

bool Physics::Start()
{
	LOG("Creating Physics 2D environment");

	// Create a new World
	world = new b2World(b2Vec2(GRAVITY_X, -GRAVITY_Y));

	// Set this module as a listener for contacts
	world->SetContactListener(this);

	return true;
}

// 
bool Physics::PreUpdate()
{
	bool ret = true;
	if (Engine::GetInstance().menus->currentState != MenusState::GAME) {return ret; }


	// Step (update) the World
	//Get the dt form the engine. Note that dt is in miliseconds and steps in Box2D are in seconds
	float dt = Engine::GetInstance().GetDt() / 1000; 
	world->Step(dt, 6, 2);

	// Because Box2D does not automatically broadcast collisions/contacts with sensors, 
	// we have to manually search for collisions and "call" the equivalent to the ModulePhysics::BeginContact() ourselves...
	for (b2Contact* c = world->GetContactList(); c; c = c->GetNext())
	{
		// For each contact detected by Box2D, see if the first one colliding is a sensor
		if (c->IsTouching() && c->GetFixtureA()->IsSensor())
		{
			// If so, we call the OnCollision listener function (only of the sensor), passing as inputs our custom PhysBody classes
			PhysBody* pb1 = (PhysBody*)c->GetFixtureA()->GetBody()->GetUserData().pointer;
			PhysBody* pb2 = (PhysBody*)c->GetFixtureB()->GetBody()->GetUserData().pointer;
			
			if (pb1 && pb2 && pb1->listener && !IsPendingToDelete(pb1) && !IsPendingToDelete(pb2))
				pb1->listener->OnCollision(pb1, pb2);
		}
	}

	return ret;
}

PhysBody* Physics::CreateRectangle(int x, int y, int width, int height, bodyType type, float offsetX, float offsetY, uint16 categoryBits, uint16 maskBits)
{
	b2BodyDef body;
	if (type == DYNAMIC) body.type = b2_dynamicBody;
	if (type == STATIC) body.type = b2_staticBody;
	if (type == KINEMATIC) body.type = b2_kinematicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	b2Body* b = world->CreateBody(&body);
	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f,
		b2Vec2(PIXEL_TO_METERS(offsetX), PIXEL_TO_METERS(offsetY)), 0);

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	fixture.restitution = 0.0f;
	fixture.friction = 0.0f;
	fixture.filter.categoryBits = categoryBits;
	fixture.filter.maskBits = maskBits;

	b->SetFixedRotation(true);
	b->ResetMassData();
	b->CreateFixture(&fixture);

	PhysBody* pbody = new PhysBody();
	pbody->body = b;
	b->GetUserData().pointer = (uintptr_t)pbody;
	pbody->width = width * 0.5f;
	pbody->height = height * 0.5f;

	return pbody;
}

PhysBody* Physics::CreateCircle(int x, int y, int radious, bodyType type)
{
	// Create BODY at position x,y
	b2BodyDef body;

	if (type == DYNAMIC) body.type = b2_dynamicBody;
	if (type == STATIC) body.type = b2_staticBody;
	if (type == KINEMATIC) body.type = b2_kinematicBody;

	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	// Add BODY to the world
	b2Body* b = world->CreateBody(&body);

	// Create SHAPE
	b2CircleShape circle;
	circle.m_radius = PIXEL_TO_METERS(radious);

	// Create FIXTURE
	b2FixtureDef fixture;
	fixture.shape = &circle;
	fixture.density = 1.0f;
	fixture.restitution = 0.0f;
	b->ResetMassData();

	// Add fixture to the BODY
	b->CreateFixture(&fixture);

	// Create our custom PhysBody class
	PhysBody* pbody = new PhysBody();
	pbody->body = b;
	b->GetUserData().pointer = (uintptr_t)pbody;
	pbody->width = radious * 0.5f;
	pbody->height = radious * 0.5f;

	// Return our PhysBody class
	return pbody;
}
PhysBody* Physics::CreateCircleSensor(int x, int y, int radius, bodyType type, uint16 categoryBits, uint16 maskBits)
{
	b2BodyDef body;
	if (type == DYNAMIC) body.type = b2_dynamicBody;
	if (type == STATIC) body.type = b2_staticBody;
	if (type == KINEMATIC) body.type = b2_kinematicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	b2Body* b = world->CreateBody(&body);

	b2CircleShape circle;
	circle.m_radius = PIXEL_TO_METERS(radius);

	b2FixtureDef fixture;
	fixture.shape = &circle;
	fixture.isSensor = true;
	fixture.density = 1.0f;
	fixture.filter.categoryBits = categoryBits;
	fixture.filter.maskBits = maskBits;

	b->SetFixedRotation(true);
	b->CreateFixture(&fixture);

	PhysBody* pbody = new PhysBody();
	pbody->body = b;
	b->GetUserData().pointer = (uintptr_t)pbody;
	pbody->width = radius;
	pbody->height = radius;

	return pbody;
}

PhysBody* Physics::CreateRectangleSensor(int x, int y, int width, int height, bodyType type, uint16 categoryBits, uint16 maskBits)
{
	b2BodyDef body;
	if (type == DYNAMIC) body.type = b2_dynamicBody;
	if (type == STATIC) body.type = b2_staticBody;
	if (type == KINEMATIC) body.type = b2_kinematicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	b2Body* b = world->CreateBody(&body);

	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.isSensor = true;
	fixture.density = 1.0f;

	fixture.filter.categoryBits = categoryBits;
	fixture.filter.maskBits = maskBits;

	b->SetFixedRotation(true);
	b->CreateFixture(&fixture);

	PhysBody* pbody = new PhysBody();
	pbody->body = b;
	b->GetUserData().pointer = (uintptr_t)pbody;
	pbody->width = width;
	pbody->height = height;

	return pbody;
}
PhysBody* Physics::CreatePolygon(int x, int y, const std::vector<b2Vec2>& vertices, bodyType type)
{
	b2BodyDef bodyDef;
	bodyDef.type = (type == DYNAMIC) ? b2_dynamicBody : (type == STATIC) ? b2_staticBody : b2_kinematicBody;
	bodyDef.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	b2Body* body = world->CreateBody(&bodyDef);

	b2PolygonShape shape;
	shape.Set(&vertices[0], vertices.size());

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &shape;
	fixtureDef.density = 1.0f;

	body->SetFixedRotation(true);
	body->CreateFixture(&fixtureDef);

	PhysBody* pbody = new PhysBody();
	pbody->body = body;
	body->GetUserData().pointer = (uintptr_t)pbody;
	pbody->width = 0;
	pbody->height = 0;

	return pbody;
}

PhysBody* Physics::CreateChain(int x, int y, int* points, int size, bodyType type)
{
	// Create BODY at position x,y
	b2BodyDef body;
	if (type == DYNAMIC) body.type = b2_dynamicBody;
	if (type == STATIC) body.type = b2_staticBody;
	if (type == KINEMATIC) body.type = b2_kinematicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	// Add BODY to the world
	b2Body* b = world->CreateBody(&body);

	// Create SHAPE
	b2ChainShape shape;
	b2Vec2* p = new b2Vec2[size / 2];
	for (unsigned int i = 0; i < size / 2; ++i)
	{
		p[i].x = PIXEL_TO_METERS(points[i * 2 + 0]);
		p[i].y = PIXEL_TO_METERS(points[i * 2 + 1]);
	}
	shape.CreateLoop(p, size / 2);

	// Create FIXTURE
	b2FixtureDef fixture;
	fixture.shape = &shape;

	// Add fixture to the BODY
	b->CreateFixture(&fixture);

	// Clean-up temp array
	delete p;

	// Create our custom PhysBody class
	PhysBody* pbody = new PhysBody();
	pbody->body = b;
	b->GetUserData().pointer = (uintptr_t)pbody;
	pbody->width = pbody->height = 0;

	// Return our PhysBody class
	return pbody;
}

// 
bool Physics::PostUpdate()
{
	bool ret = true;
	if (Engine::GetInstance().menus->isPaused || Engine::GetInstance().menus->currentState == MenusState::MAINMENU)
	{
		return ret;
	}

	// Activate or deactivate debug mode
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;
	
	//  Iterate all objects in the world and draw the bodies
	if (debug)
	{
		for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
		{
			for (b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
			{
				switch (f->GetType())
				{
					// Draw circles ------------------------------------------------
				case b2Shape::e_circle:
				{
					b2CircleShape* shape = (b2CircleShape*)f->GetShape();
					int width, height;
					Engine::GetInstance().window.get()->GetWindowSize(width, height);
					b2Vec2 pos = f->GetBody()->GetPosition();
					Engine::GetInstance().render.get()->DrawCircle(METERS_TO_PIXELS(pos.x), 
																   METERS_TO_PIXELS(pos.y), 
																   METERS_TO_PIXELS(shape->m_radius) * Engine::GetInstance().window.get()->GetScale(),
																   255, 255, 255);
				}
				break;

				// Draw polygons ------------------------------------------------
				case b2Shape::e_polygon:
				{
					b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();
					int32 count = polygonShape->m_count;
					b2Vec2 prev, v;

					for (int32 i = 0; i < count; ++i)
					{
						v = b->GetWorldPoint(polygonShape->m_vertices[i]);
						if (i > 0)
							Engine::GetInstance().render.get()->DrawLine(METERS_TO_PIXELS(prev.x), 
																	     METERS_TO_PIXELS(prev.y), 
																		 METERS_TO_PIXELS(v.x), 
																		 METERS_TO_PIXELS(v.y), 
																		 255, 255, 100);

						prev = v;
					}

					v = b->GetWorldPoint(polygonShape->m_vertices[0]);
					Engine::GetInstance().render.get()->DrawLine(METERS_TO_PIXELS(prev.x),
																 METERS_TO_PIXELS(prev.y),
																 METERS_TO_PIXELS(v.x),
																 METERS_TO_PIXELS(v.y),
																 255, 255, 100);
				}
				break;

				// Draw chains contour -------------------------------------------
				case b2Shape::e_chain:
				{
					b2ChainShape* shape = (b2ChainShape*)f->GetShape();
					b2Vec2 prev, v;

					for (int32 i = 0; i < shape->m_count; ++i)
					{
						v = b->GetWorldPoint(shape->m_vertices[i]);
						if (i > 0)
							Engine::GetInstance().render.get()->DrawLine(METERS_TO_PIXELS(prev.x), 
																	     METERS_TO_PIXELS(prev.y), 
																	     METERS_TO_PIXELS(v.x), 
																	     METERS_TO_PIXELS(v.y), 
																		 100, 255, 100);
						prev = v;
					}

					v = b->GetWorldPoint(shape->m_vertices[0]);
					Engine::GetInstance().render.get()->DrawLine(METERS_TO_PIXELS(prev.x), 
																 METERS_TO_PIXELS(prev.y), 
															     METERS_TO_PIXELS(v.x), 
															     METERS_TO_PIXELS(v.y), 
																 100, 255, 100);
				}
				break;

				// Draw a single segment(edge) ----------------------------------
				case b2Shape::e_edge:
				{
					b2EdgeShape* shape = (b2EdgeShape*)f->GetShape();
					b2Vec2 v1, v2;

					v1 = b->GetWorldPoint(shape->m_vertex0);
					v1 = b->GetWorldPoint(shape->m_vertex1);
					Engine::GetInstance().render.get()->DrawLine(METERS_TO_PIXELS(v1.x), 
																 METERS_TO_PIXELS(v1.y), 
																 METERS_TO_PIXELS(v2.x), 
																 METERS_TO_PIXELS(v2.y), 
															     100, 100, 255);
				}
				break;
				}

			}
		}
	}

	// Process bodies to delete after the world step
	for (PhysBody* physBody : bodiesToDelete) {
		if (physBody && physBody->body) {
			world->DestroyBody(physBody->body);
			physBody->body = nullptr;
		}
	}
	bodiesToDelete.clear();

	return ret;
}

// Called before quitting
bool Physics::CleanUp()
{
	LOG("Destroying physics world");

	// Delete the whole physics world!
	delete world;

	return true;
}

// Callback function to collisions with Box2D
void Physics::BeginContact(b2Contact* contact)
{
	// Call the OnCollision listener function to bodies A and B, passing as inputs our custom PhysBody classes
	PhysBody* physA = (PhysBody*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
	PhysBody* physB = (PhysBody*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

	if (physA && physA->listener != NULL && !IsPendingToDelete(physA)) {
		if (physB) // Ensure physB is also valid
		{
			physA->listener->OnCollision(physA, physB);
		}
	}

	if (physB && physB->listener != NULL && !IsPendingToDelete(physB)) {
		if(physA) // Ensure physA is also valid
		{
			physB->listener->OnCollision(physB, physA);
		}
	}
}

// Callback function to collisions with Box2D
void Physics::EndContact(b2Contact* contact)
{
	// Call the OnCollision listener function to bodies A and B, passing as inputs our custom PhysBody classes
	PhysBody* physA = (PhysBody*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
	PhysBody* physB = (PhysBody*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

	if (physA && physA->listener != NULL && !IsPendingToDelete(physA)) {
		if (physB) // Ensure physB is also valid
		{
			physA->listener->OnCollisionEnd(physA, physB);
		}
	}

	if (physB && physB->listener != NULL && !IsPendingToDelete(physB)) {
		if (physA) // Ensure physA is also valid
		{
			physB->listener->OnCollisionEnd(physB, physA);
		}
	}
}

bool Physics::IsPendingToDelete(PhysBody* physBody) {
	bool pendingToDelete = false;
	for (PhysBody* _physBody : bodiesToDelete) {
		if (_physBody == physBody) {
			pendingToDelete = true;
			break;
		}
	}
	return pendingToDelete;
}

void Physics::DeletePhysBody(PhysBody* physBody) {
	if (!IsPendingToDelete(physBody)) {
		bodiesToDelete.push_back(physBody);
	}
}

//--------------- PhysBody

void PhysBody::GetPosition(int& x, int& y) const
{
	b2Vec2 pos = body->GetPosition();
	x = METERS_TO_PIXELS(pos.x) - (width);
	y = METERS_TO_PIXELS(pos.y) - (height);
}

float PhysBody::GetRotation() const
{
	return RADTODEG * body->GetAngle();
}

bool PhysBody::Contains(int x, int y) const
{
	b2Vec2 p(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	const b2Fixture* fixture = body->GetFixtureList();

	while (fixture != NULL)
	{
		if (fixture->GetShape()->TestPoint(body->GetTransform(), p) == true)
			return true;
		fixture = fixture->GetNext();
	}

	return false;
}

int PhysBody::RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const
{
	int ret = -1;

	b2RayCastInput input;
	b2RayCastOutput output;

	input.p1.Set(PIXEL_TO_METERS(x1), PIXEL_TO_METERS(y1));
	input.p2.Set(PIXEL_TO_METERS(x2), PIXEL_TO_METERS(y2));
	input.maxFraction = 1.0f;

	const b2Fixture* fixture = body->GetFixtureList();

	while (fixture != NULL)
	{
		if (fixture->GetShape()->RayCast(&output, input, body->GetTransform(), 0) == true)
		{
			// do we want the normal ?

			float fx = x2 - x1;
			float fy = y2 - y1;
			float dist = sqrtf((fx * fx) + (fy * fy));

			normal_x = output.normal.x;
			normal_y = output.normal.y;

			return output.fraction * dist;
		}
		fixture = fixture->GetNext();
	}

	return ret;
}
void Physics::FlipPhysBody(PhysBody* body, bool horizontal, bool vertical)
{
	if (body == nullptr || body->body == nullptr)
		return;

	b2Body* b2body = body->body;

	for (b2Fixture* f = b2body->GetFixtureList(); f != nullptr; f = f->GetNext())
	{
		b2Shape::Type type = f->GetType();
		b2Shape* shape = f->GetShape();

		switch (type)
		{
		case b2Shape::e_polygon:
		{
			b2PolygonShape* polygon = (b2PolygonShape*)shape;
			b2Vec2 flippedVerts[b2_maxPolygonVertices];

			for (int i = 0; i < polygon->m_count; ++i)
			{
				flippedVerts[i] = polygon->m_vertices[i];
				if (horizontal)
					flippedVerts[i].x *= -1;
				if (vertical)
					flippedVerts[i].y *= -1;
			}

			// Reemplazar fixture
			b2FixtureDef fixtureDef;
			fixtureDef.shape = nullptr;
			fixtureDef.density = f->GetDensity();
			fixtureDef.friction = f->GetFriction();
			fixtureDef.restitution = f->GetRestitution();
			fixtureDef.isSensor = f->IsSensor();
			fixtureDef.filter = f->GetFilterData();

			b2PolygonShape newShape;
			newShape.Set(flippedVerts, polygon->m_count);
			fixtureDef.shape = &newShape;

			b2body->DestroyFixture(f);
			b2body->CreateFixture(&fixtureDef);
			break;
		}
		case b2Shape::e_circle:
		{
			// Para círculos, solo necesitamos reflejar el centro si no está centrado
			b2CircleShape* circle = (b2CircleShape*)shape;
			b2Vec2 pos = circle->m_p;

			if (horizontal) pos.x *= -1;
			if (vertical) pos.y *= -1;

			b2CircleShape newCircle;
			newCircle.m_radius = circle->m_radius;
			newCircle.m_p = pos;

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &newCircle;
			f->GetFilterData(); // Mantener filtros
			fixtureDef.density = f->GetDensity();
			fixtureDef.friction = f->GetFriction();
			fixtureDef.restitution = f->GetRestitution();
			fixtureDef.isSensor = f->IsSensor();
			fixtureDef.filter = f->GetFilterData();

			b2body->DestroyFixture(f);
			b2body->CreateFixture(&fixtureDef);
			break;
		}
		default:
			LOG("FlipPhysBody: Shape type not supported for flipping.");
			break;
		}
	}

	// También puedes invertir la escala visual si tienes sprites ligados
}

