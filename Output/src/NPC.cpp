#include "Engine.h"
#include "Physics.h"
#include "Module.h"
#include "NPC.h"
#include "Textures.h"
#include "DialogueManager.h"
#include "Animation.h"
#include "SDL2/SDL.h"
#include "pugixml.hpp"

NPC::NPC(EntityType type) : Entity(type){
}

NPC::~NPC() {
}

bool NPC::Awake() {
	return true;
}

bool NPC::Start() {
	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + width/2, (int)position.getY()+height/2, width, height, bodyType::STATIC, CATEGORY_NPC, CATEGORY_PLAYER);

	pbody->ctype = ColliderType::NPC;

	pbody->listener = this;

	if (!gravity) pbody->body->SetGravityScale(0);

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");
	for (char& c : type)
		c = std::tolower(static_cast<unsigned char>(c));
	pugi::xml_node npcNode = loadFile.child("config").child("scene").child("animations").child("npcs").child(type.c_str());

	// Cargar textura y animaciones
	texture = Engine::GetInstance().textures->Load(npcNode.attribute("texture").as_string());
	texH = npcNode.attribute("h").as_int();
	texW = npcNode.attribute("w").as_int();
	idleAnim.LoadAnimations(npcNode.child("idle"));

	currentAnimation = &idleAnim;

	return true;
}

bool NPC::Update(float dt)
{
	
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - width / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - height / 2);

	if (type.c_str() == "BichoPalo");
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + width/2 - texW/2, (int)position.getY() + height - texH, &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();

	return true;
}

bool NPC::PostUpdate()
{
	return true;
}

bool NPC::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void NPC::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + width / 2);
	pos.setY(pos.getY() + height / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D NPC::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void NPC::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		Engine::GetInstance().dialogueManager.get()->SetDialogueAvailable(dialogueId, Vector2D(GetPosition().x - texW/2, GetPosition().y - texH), true);
		break;
	}
}

void NPC::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		Engine::GetInstance().dialogueManager.get()->SetDialogueAvailable(dialogueId, Vector2D(GetPosition().x - texW / 2, GetPosition().y - texH), false);
		break;
	}
		
}