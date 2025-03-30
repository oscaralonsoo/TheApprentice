#include "CaveDrop.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

CaveDrop::CaveDrop() : Entity(EntityType::CAVEDROP)
{
	name = "CaveDrop";
}

CaveDrop::~CaveDrop() {}

bool CaveDrop::Awake() {
	return true;
}

bool CaveDrop::Start() {

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");
	pugi::xml_node caveDropNode = loadFile.child("config").child("scene").child("animations").child("props").child("cave_drop");
	texture = Engine::GetInstance().textures.get()->Load(caveDropNode.attribute("texture").as_string());
	startAnim.LoadAnimations(caveDropNode.child("start"));
	
	Engine::GetInstance().textures.get()->GetSize(texture, texW, texH);
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::STATIC);

	currentAnimation = &startAnim;

	pbody->ctype = ColliderType::CAVEDROP;

	return true;
}

bool CaveDrop::Update(float dt)
{
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();
	return true;
}

bool CaveDrop::CleanUp()
{
	return true;
}