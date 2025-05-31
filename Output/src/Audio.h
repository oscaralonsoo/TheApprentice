#pragma once

#include "Module.h"
#include "SDL2/SDL_mixer.h"
#include <list>
#include "pugixml.hpp"
#include "pugiconfig.hpp"

#define DEFAULT_MUSIC_FADE_TIME 2.0f

struct _Mix_Music;

class Audio : public Module
{
public:

	Audio();

	// Destructor
	virtual ~Audio();

	// Called before render is available
	bool Awake();

	// Called before quitting
	bool CleanUp();

	// Load a WAV in memory
	int LoadFx(const char* path, float relativeVolume);

	// Play a previously loaded WAV
	bool PlayFx(int fxId, float relativeVolume, int repeat);


	// Control de volumen
	void SetMasterVolume(float volume);    // 0.0f - 1.0f
	void SetMusicVolume(float volume);     // 0.0f - 1.0f
	void SetSfxVolume(float volume);       // 0.0f - 1.0f

	float GetMasterVolume() const { return masterVolume; }
	float GetMusicVolume() const { return musicVolume; }
	float GetSfxVolume() const { return sfxVolume; }

	// Play music con volumen relativo
	bool PlayMusic(const char* path, float fadeTime, float customVolume); // nuevo overload

	int PlayFxReturnChannel(int fxId, float relativeVolume, int repeat);

	Mix_Chunk* GetFx(int id) const;

	bool Save(pugi::xml_node& config) ;
	bool Load(pugi::xml_node& config) ;

	float GetFinalFxVolume(int id) const;
	float GetFinalMusicVolume() const;

	float masterVolume = 0.5f;
	float musicVolume = 0.5f;
	float sfxVolume = 0.5f;
private:

	_Mix_Music* music;
	std::list<Mix_Chunk*> fx;

	std::list<float> fxVolumes;
};
