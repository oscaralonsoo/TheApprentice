#include "Menus.h"
#include "Scene.h"
#include "Input.h"
#include <SDL2/SDL.h>
#include "Render.h"

Menus::Menus() : currentState(MenusState::MAINMENU), ret(false), transitionAlpha(0.0f)
{
}

Menus::~Menus()
{
}

bool Menus::Awake()
{
	return true;
}

bool Menus::Start()
{
	currentState = MenusState::MAINMENU;

	pauseMenuImage;
	return true;
}

bool Menus::PreUpdate()
{
	return true;
}

bool Menus::Update(float dt)
{
	CheckCurrentState(dt);

	if (inTransition)
	{
		Transition(dt);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
	{
		if (currentState == MenusState::PAUSE)
		{
			inTransition = true;
			fadingIn = false; 

		}
	}
	else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN && currentState == MenusState::PAUSE)
	{
		MenusState::MAINMENU;
		{
			inTransition = true;
			fadingIn = false;
		}
	}

	return true;
}

bool Menus::PostUpdate()
{
	if (transitioning)
	{
		// Dibujar un rectángulo negro con opacidad variable
		SDL_SetRenderDrawBlendMode(Engine::GetInstance().render->renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(Engine::GetInstance().render->renderer, 0, 0, 0, static_cast<Uint8>(transitionAlpha * 255));
		SDL_RenderFillRect(Engine::GetInstance().render->renderer, nullptr);
	}

	return true;
}

bool Menus::CleanUp()
{
	return true;
}

void Menus::CheckCurrentState(float dt)
{
	switch (currentState)
	{
	case MenusState::MAINMENU:
		MainMenu();
		break;
	case MenusState::NEWGAME:
		NewGame();
		break;
	case MenusState::CONTINUE:
		Continue();
		break;
	case MenusState::PAUSE:
		Pause();
		break;
	case MenusState::SETTINGS:
		Settings();
		break;
	case MenusState::CREDITS:
		Credits();
		break;
	case MenusState::EXIT:
		ret = true;
		break;
	}
}

void Menus::MainMenu()
{
}

void Menus::NewGame()
{

}

void Menus::Continue()
{
}

void Menus::Pause()
{
	/*Engine::GetInstance().render.get()->DrawTexture(pauseMenuImage,);*/
}

void Menus::Settings()
{

}

void Menus::Credits()
{

}
void Menus::Transition(float dt)
{
	// Fade Out
	if (!fadingIn)
	{
		transitionAlpha += dt * 0.0025f;
		if (transitionAlpha >= 1.0f)
		{
			transitionAlpha = 1.0f;
			fadingIn = true;
			transitioning = true; 
		}
	}
	else // Fade In
	{
		transitionAlpha += dt * 0.0020f;
		if (transitionAlpha <= 0.0f)
		{
			transitionAlpha = 0.0f;
			transitioning = false; 
			inTransition = false;  
		}
	}
}
