/**
 * Author: Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.one/)
 */
#ifndef __FLIPCLOCK_H__
#define __FLIPCLOCK_H__

#include <stdbool.h>
#include <time.h>

#include <SDL.h>
#include <SDL_ttf.h>

#ifdef _WIN32
#	include <windows.h>
#endif

/* Android APP does not generate `config.h` and use its own logger. */
#ifdef __ANDROID__
#	include <android/log.h>
#	define LOG_TAG "FlipClock"
#	ifdef __DEBUG__
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
#	ifdef __DEBUG__
#		define LOG_DEBUG(...) fprintf(stdout, __VA_ARGS__)
#	else
#		define LOG_DEBUG(...)
#	endif
#	define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
#	include "config.h"
#endif

#define PROGRAM_TITLE "FlipClock"
#define MAX_BUFFER_LENGTH 512

/* Those are from arguments. */
struct properties {
	const char *title;
	char font_path[MAX_BUFFER_LENGTH];
	bool ampm;
	bool full;
#ifdef _WIN32
	HWND preview_window;
	bool preview;
	bool screensaver;
#endif
};
struct colors {
	SDL_Color font;
	SDL_Color rect;
	SDL_Color black;
	SDL_Color transparent;
};
struct times {
	struct tm past;
	struct tm now;
};
struct fonts {
	TTF_Font *time;
	TTF_Font *mode;
};
struct rects {
	SDL_Rect hour;
	SDL_Rect minute;
	SDL_Rect mode;
};
struct textures {
	SDL_Texture *current;
	SDL_Texture *previous;
};
struct clock {
	SDL_Window *window;
	SDL_Renderer *renderer;
	struct textures textures;
	struct rects rects;
	struct fonts fonts;
	int width;
	int height;
	int rect_size;
	int radius;
	bool wait;
};
/* You only need this to create an app. */
struct flipclock {
	/* Structures not shared by clocks. */
	struct clock *clocks;
	/* Number of displays. */
	int clocks_length;
	/* Structures shared by clocks. */
	struct times times;
	struct colors colors;
	struct properties properties;
	unsigned int last_touch;
	bool running;
};

struct flipclock *flipclock_create(void);
void flipclock_load_conf(struct flipclock *app);
void flipclock_create_clocks(struct flipclock *app);
void flipclock_set_fullscreen(struct flipclock *app, int clock_index,
			      bool full);
void flipclock_refresh(struct flipclock *app, int clock_index);
void flipclock_create_textures(struct flipclock *app, int clock_index);
void flipclock_destroy_textures(struct flipclock *app, int clock_index);
void flipclock_open_fonts(struct flipclock *app, int clock_index);
void flipclock_close_fonts(struct flipclock *app, int clock_index);
void flipclock_clear_texture(struct flipclock *app, int clock_index,
			     SDL_Texture *target_texture,
			     SDL_Color background_color);
void flipclock_render_rounded_box(struct flipclock *app, int clock_index,
				  SDL_Texture *target_texture,
				  SDL_Rect target_rect, int radius);
void flipclock_render_text(struct flipclock *app, int clock_index,
			   SDL_Texture *target_texture, SDL_Rect target_rect,
			   TTF_Font *font, char text[]);
void flipclock_render_divider(struct flipclock *app, int clock_index,
			      SDL_Texture *target_texture,
			      SDL_Rect target_rect);
void flipclock_render_texture(struct flipclock *app, int clock_index);
void flipclock_copy_rect(struct flipclock *app, int clock_index,
			 SDL_Rect target_rect, int progress);
void flipclock_animate(struct flipclock *app, int clock_index, int progress);
void flipclock_handle_window_event(struct flipclock *app, SDL_Event event);
void flipclock_handle_event(struct flipclock *app, SDL_Event event);
void flipclock_run_mainloop(struct flipclock *app);
void flipclock_destroy_clocks(struct flipclock *app);
void flipclock_destroy(struct flipclock *app);
void flipclock_print_help(char program_name[]);

#endif
