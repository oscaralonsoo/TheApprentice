#include "Audio.h"
#include "Log.h"
#include "Engine.h"
#include "Scene.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

template<typename T>
T Clamp(T value, T min, T max)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

Audio::Audio() : Module()
{
	music = NULL;
	name = "audio";
}

Audio::~Audio() {}

bool Audio::Awake()
{
	LOG("Loading Audio Mixer");
	bool ret = true;
	SDL_Init(0);

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		LOG("SDL_INIT_AUDIO could not initialize! SDL_Error: %s\n", SDL_GetError());
		active = false;
		ret = true;
	}

	int flags = MIX_INIT_OGG;
	int init = Mix_Init(flags);

	if ((init & flags) != flags)
	{
		LOG("Could not initialize Mixer lib. Mix_Init: %s", Mix_GetError());
		active = false;
		ret = true;
	}

	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		LOG("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		active = false;
		ret = true;
	}

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	pugi::xml_node configParameters = loadFile.child("config"); 
	pugi::xml_node audioConfig = configParameters.child("audio");

	if (!audioConfig.empty())
	{
		masterVolume = Clamp(audioConfig.child("master").attribute("value").as_float(1.0f), 0.0f, 1.0f);
		musicVolume = Clamp(audioConfig.child("music").attribute("value").as_float(1.0f), 0.0f, 1.0f);
		sfxVolume = Clamp(audioConfig.child("sfx").attribute("value").as_float(1.0f), 0.0f, 1.0f);
	}

	// Aplica los volúmenes ya cargados
	Mix_VolumeMusic(static_cast<int>(musicVolume * masterVolume * MIX_MAX_VOLUME));
	Mix_Volume(-1, static_cast<int>(sfxVolume * masterVolume * MIX_MAX_VOLUME));

	return ret;
}

bool Audio::CleanUp()
{
	if (!active)
		return true;

	LOG("Freeing sound FX, closing Mixer and Audio subsystem");
	pugi::xml_node audioConfig = configParameters.child("audio");

	audioConfig.child("master").attribute("value") = masterVolume;
	audioConfig.child("music").attribute("value") = musicVolume;
	audioConfig.child("sfx").attribute("value") = sfxVolume;
	if (music != NULL)
	{
		Mix_FreeMusic(music);
	}

	for (const auto& fxItem : fx) {
		Mix_FreeChunk(fxItem);
	}
	fx.clear();
	fxVolumes.clear();

	Mix_CloseAudio();
	Mix_Quit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	return true;
}

bool Audio::PlayMusic(const char* path, float fadeTime, float customVolume)
{
	if (!active) return false;

	currentMusicPath = path;

	if (music != NULL) {
		if (fadeTime > 0.0f)
			Mix_FadeOutMusic(int(fadeTime * 1000.0f));
		else
			Mix_HaltMusic();

		Mix_FreeMusic(music);
	}

	music = Mix_LoadMUS(path);
	if (music == NULL)
	{
		LOG("Cannot load music %s. Mix_GetError(): %s\n", path, Mix_GetError());
		return false;
	}

	int finalVolume = int(MIX_MAX_VOLUME * Clamp(customVolume, 0.0f, 1.0f) * musicVolume * masterVolume);
	Mix_VolumeMusic(finalVolume);

	if (fadeTime > 0.0f)
		Mix_FadeInMusic(music, -1, int(fadeTime * 1000.0f));
	else
		Mix_PlayMusic(music, -1);

	LOG("Successfully playing %s", path);
	return true;
}

int Audio::LoadFx(const char* path, float relativeVolume)
{
	if (!active) return 0;

	Mix_Chunk* chunk = Mix_LoadWAV(path);
	if (!chunk)
	{
		LOG("Cannot load wav %s. Mix_GetError(): %s", path, Mix_GetError());
		return 0;
	}

	fx.push_back(chunk);
	fxVolumes.push_back(Clamp(relativeVolume, 0.0f, 1.0f));
	return fx.size();
}

bool Audio::PlayFx(int id, float relativeVolume, int repeat)
{
	if (!active || id <= 0 || id > fx.size()) return false;

	auto fxIt = fx.begin();
	auto volIt = fxVolumes.begin();
	std::advance(fxIt, id - 1);
	std::advance(volIt, id - 1);

	// Aplica tanto el volumen cargado como el pasado por parámetro
	float combinedVolume = Clamp(relativeVolume * (*volIt), 0.0f, 1.0f);
	int finalVolume = int(MIX_MAX_VOLUME * combinedVolume * sfxVolume * masterVolume);

	Mix_VolumeChunk(*fxIt, finalVolume);
	Mix_PlayChannel(-1, *fxIt, repeat);

	return true;
}

void Audio::SetMasterVolume(float volume)
{
	masterVolume = Clamp(volume, 0.0f, 1.0f);
	Mix_VolumeMusic(int(MIX_MAX_VOLUME * musicVolume * masterVolume));
}

void Audio::SetMusicVolume(float volume)
{
	musicVolume = Clamp(volume, 0.0f, 1.0f);
	Mix_VolumeMusic(int(MIX_MAX_VOLUME * musicVolume * masterVolume));
}

void Audio::SetSfxVolume(float volume)
{
	sfxVolume = Clamp(volume, 0.0f, 1.0f);
}

Mix_Chunk* Audio::GetFx(int id) const
{
	if (id <= 0 || id > fx.size()) return nullptr;
	auto it = fx.begin();
	std::advance(it, id - 1);
	return *it;
}
bool Audio::Save(pugi::xml_node& config)
{
	pugi::xml_node audioConfig = config.append_child("audio");

	audioConfig.append_child("master").append_attribute("value") = masterVolume;
	audioConfig.append_child("music").append_attribute("value") = musicVolume;
	audioConfig.append_child("sfx").append_attribute("value") = sfxVolume;

	return true;
}
bool Audio::Load(pugi::xml_node& config)
{
	pugi::xml_node audioConfig = config.child("audio");
	if (!audioConfig.empty())
	{
		masterVolume = Clamp(audioConfig.child("master").attribute("value").as_float(1.0f), 0.0f, 1.0f);
		musicVolume = Clamp(audioConfig.child("music").attribute("value").as_float(1.0f), 0.0f, 1.0f);
		sfxVolume = Clamp(audioConfig.child("sfx").attribute("value").as_float(1.0f), 0.0f, 1.0f);
	}

	return true;
}
float Audio::GetFinalFxVolume(int id) const
{
	if (id <= 0 || id > fx.size()) return 0.0f;

	auto volIt = fxVolumes.begin();
	std::advance(volIt, id - 1);
	return Clamp((*volIt) * sfxVolume * masterVolume, 0.0f, 1.0f);
}

float Audio::GetFinalMusicVolume() const
{
	return Clamp(musicVolume * masterVolume, 0.0f, 1.0f);
}

int Audio::PlayFxReturnChannel(int id, float relativeVolume, int repeat)
{
	if (!active || id <= 0 || id > fx.size()) return -1;

	auto fxIt = fx.begin();
	auto volIt = fxVolumes.begin();
	std::advance(fxIt, id - 1);
	std::advance(volIt, id - 1);

	float combinedVolume = Clamp(relativeVolume * (*volIt), 0.0f, 1.0f);
	int finalVolume = int(MIX_MAX_VOLUME * combinedVolume * sfxVolume * masterVolume);
	Mix_VolumeChunk(*fxIt, finalVolume);

	return Mix_PlayChannel(-1, *fxIt, repeat);
}
void Audio::StopMusic()
{
	if (!active || music == nullptr)
		return;

	Mix_HaltMusic();
	Mix_FreeMusic(music);
	music = nullptr;
}