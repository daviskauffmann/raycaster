#include <SDL/SDL_mixer.h>
#include <SDL/SDL.h>

#include "audio.h"

#define FLAGS 0
#define FREQUENCY 44100
#define FORMAT MIX_DEFAULT_FORMAT
#define CHANNELS 2
#define CHUNK_SIZE 1024

int audio_init(void)
{
    if ((Mix_Init(FLAGS) & FLAGS) != FLAGS)
    {
        SDL_Log("Mix_Init: %s", Mix_GetError());

        return 1;
    }

    if (Mix_OpenAudio(FREQUENCY, FORMAT, CHANNELS, CHUNK_SIZE) != 0)
    {
        SDL_Log("Mix_OpenAudio: %s", Mix_GetError());

        return 1;
    }

    SDL_Log("Audio initialized");

    return 0;
}

Mix_Music *audio_load_music(const char *file)
{
    return Mix_LoadMUS(file);
}

Mix_Chunk *audio_load_chunk(const char *file)
{
    return Mix_LoadWAV(file);
}

void audio_play_music(Mix_Music *music, int loops)
{
    Mix_PlayMusic(music, loops);
}

int audio_playing_music()
{
    return Mix_PlayingMusic();
}

void audio_pause_music(void)
{
    Mix_PauseMusic();
}

int audio_paused_music(void)
{
    return Mix_PausedMusic();
}

void audio_resume_music(void)
{
    Mix_ResumeMusic();
}

void audio_stop_music(void)
{
    Mix_HaltMusic();
}

void audio_play_chunk(Mix_Chunk *chunk, int loops)
{
    Mix_PlayChannel(-1, chunk, loops);
}

void audio_unload_music(Mix_Music *music)
{
    Mix_FreeMusic(music);
}

void audio_unload_chunk(Mix_Chunk *chunk)
{
    Mix_FreeChunk(chunk);
}

void audio_quit(void)
{
    Mix_CloseAudio();
    Mix_Quit();
}
