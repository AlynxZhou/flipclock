/*
 * Author: Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.moe/)
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

#define PROGRAM_TITLE "FlipClock"

struct properties {
	const char *title;
	const char *font_path;
	bool ampm;
	bool full;
	int width;
	int height;
	int rect_size;
	int radius;
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
/* You only need this to create an app. */
struct flipclock {
	SDL_Window *window;
	SDL_Renderer *renderer;
	struct textures textures;
	struct rects rects;
	struct fonts fonts;
	struct times times;
	struct colors colors;
	struct properties properties;
};

struct flipclock *flipclock_create(void);
void flipclock_create_window(struct flipclock *app);
void flipclock_set_fullscreen(struct flipclock *app, bool full);
void flipclock_refresh(struct flipclock *app);
void flipclock_create_textures(struct flipclock *app);
void flipclock_destroy_textures(struct flipclock *app);
void flipclock_open_fonts(struct flipclock *app);
void flipclock_close_fonts(struct flipclock *app);
void flipclock_clear_texture(struct flipclock *app, SDL_Texture *target_texture,
			     SDL_Color background_color);
void flipclock_render_rounded_box(struct flipclock *app,
				  SDL_Texture *target_texture,
				  SDL_Rect target_rect, int radius);
void flipclock_render_text(struct flipclock *app, SDL_Texture *target_texture,
			   SDL_Rect target_rect, TTF_Font *font, char text[]);
void flipclock_render_divider(struct flipclock *app,
			      SDL_Texture *target_texture,
			      SDL_Rect target_rect);
void flipclock_render_texture(struct flipclock *app);
void flipclock_copy_rect(struct flipclock *app, SDL_Rect target_rect,
			 int progress);
void flipclock_animate(struct flipclock *app, int progress);
void flipclock_run_mainloop(struct flipclock *app);
void flipclock_destroy(struct flipclock *app);
void flipclock_destroy_window(struct flipclock *app);
void flipclock_print_help(char program_name[]);

#endif
