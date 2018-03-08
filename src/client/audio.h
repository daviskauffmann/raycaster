#ifndef AUDIO_H
#define AUDIO_H

#include <SDL\SDL_mixer.h>
#include <SDL\SDL.h>

void audio_init(void);
Mix_Music *audio_load_music(const char *file);
Mix_Chunk *audio_load_chunk(const char *file);
void audio_play_music(Mix_Music *music, int loops);
int audio_playing_music(void);
void audio_pause_music(void);
int audio_paused_music(void);
void audio_resume_music(void);
void audio_stop_music(void);
void audio_play_chunk(Mix_Chunk *chunk, int loops);
void audio_unload_music(Mix_Music *music);
void audio_unload_chunk(Mix_Chunk *chunk);
void audio_quit(void);

#endif
