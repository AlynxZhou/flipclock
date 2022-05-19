/**
 * Author: Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.one/)
 */
#ifndef __FLIPCLOCK_H__
#define __FLIPCLOCK_H__

#include <stdbool.h>
#include <time.h>

#include <SDL.h>
#include <SDL_ttf.h>

#if defined(_WIN32)
#	include <windows.h>
#endif

// Android APP does not generate `config.h` and use its own logger.
#if defined(__ANDROID__)
#	include <android/log.h>
#	define LOG_TAG "FlipClock"
#	if defined(__DEBUG__)
#		define LOG_DEBUG(...)                                  \
			__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, \
					    __VA_ARGS__)
#	else
#		define LOG_DEBUG(...)
#	endif
#	define LOG_ERROR(...) \
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#	include <stdio.h>
#	if defined(__DEBUG__)
#		define LOG_DEBUG(...) fprintf(stdout, __VA_ARGS__)
#	else
#		define LOG_DEBUG(...)
#	endif
#	define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
#	include "config.h"
#endif

#define PROGRAM_TITLE "FlipClock"
#define MAX_BUFFER_LENGTH 2048

struct flipclock {
	// Structures not shared by clocks.
	struct flipclock_clock **clocks;
	// Number of clocks.
	int clocks_length;
	// Structures shared by clocks.
	struct tm now;
	SDL_Color box_color;
	SDL_Color text_color;
	SDL_Color background_color;
	char font_path[MAX_BUFFER_LENGTH];
	char conf_path[MAX_BUFFER_LENGTH];
	double text_scale;
	double card_scale;
#if defined(_WIN32)
	HWND preview_window;
	bool preview;
	bool screensaver;
	char program_dir[MAX_BUFFER_LENGTH];
#endif
	bool ampm;
	bool full;
	long long last_touch;
	bool running;
};

struct flipclock *flipclock_create(void);
void flipclock_load_conf(struct flipclock *app);
void flipclock_create_clocks(struct flipclock *app);
void flipclock_refresh(struct flipclock *app, int clock_index);
void flipclock_create_textures(struct flipclock *app, int clock_index);
void flipclock_destroy_textures(struct flipclock *app, int clock_index);
void flipclock_open_fonts(struct flipclock *app, int clock_index);
void flipclock_close_fonts(struct flipclock *app, int clock_index);
void flipclock_run_mainloop(struct flipclock *app);
void flipclock_destroy_clocks(struct flipclock *app);
void flipclock_destroy(struct flipclock *app);
void flipclock_print_help(struct flipclock *app, char program_name[]);

#endif
