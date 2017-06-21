/*
 * Filename: flipclock.h
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 */
#ifndef __FLIPCLOCK_H__
#	define __FILPCLOCK_H__

#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>
#	include <stdbool.h>
#	include <time.h>

#	include "SDL2/SDL.h"
#	include "SDL2/SDL_ttf.h"
#	include "getarg.h"

#	define MAX_STEPS 180
#	define TIMEOUT 200

struct app_properties {
	const char *title;
	const char *version;
	const char *program_name;
	const char *font_path;
	const char *fallback_font;
	bool full;
	bool ampm;
	int width;
	int height;
	double scale;
	int rect_size;
	int width_space;
	int time_radius;
	int mode_radius;
};
struct app_colors {
	SDL_Color font;
	SDL_Color rect;
	SDL_Color black;
	SDL_Color transparent;
};
struct app_times {
	struct tm past;
	struct tm now;
};
struct app_fonts {
	TTF_Font *time;
	TTF_Font *mode;
};
struct app_rects {
	SDL_Rect hour;
	SDL_Rect minute;
	SDL_Rect mode;
};
struct app_textures {
	SDL_Texture *texture;
	SDL_Texture *current;
	SDL_Texture *previous;
};
/* You only need this to create an app. */
struct app_all {
	SDL_Window *window;
	SDL_Renderer *renderer;
	struct app_textures textures;
	struct app_rects rects;
	struct app_fonts fonts;
	struct app_times times;
	struct app_colors colors;
	struct app_properties properties;
};

/*
 * Init some runtime varibles of app.
 */
bool init_app(struct app_all *app);
/*
 * Check and toggle fullscreen if needed.
 */
void toggle_fullscreen(struct app_all *app);
/*
 * Load some extras which need to calculate the size
 * and need to reload due to window resizing.
 */
bool load_extra(struct app_all *app);
/*
 * Fill default content.
 */
void fill_default(struct app_all *app);
/*
 * Clear texture with given color.
 * Use NULL to clear renderer.
 */
void clear_background(struct app_all *app, \
		      SDL_Texture *target_texture, \
		      const SDL_Color background_color);
/*
 * Clear and refresh.
 * If no clear, in Windows the DirectX will
 * let the window content undefinded.
 */
void refresh_content(struct app_all *app, \
		     int step);
/*
 * Draw a rounded box in given rect.
 * Using Bresenham's circle algorithm.
 */
void draw_rounded_box(struct app_all *app, \
		      const SDL_Rect target_rect, \
		      int radius);
/*
 * Draw a rounded box in given rect as background,
 * using draw_rounded_box().
 * Render given digits into the given rect of given texture,
 * with given fonts.
 * So this texture can be used in copy and zoom frame,
 * act as a backend.
 */
void render_time(struct app_all *app, \
		 SDL_Texture *target_texture, \
		 const SDL_Rect target_rect, \
		 TTF_Font *font, \
		 const char digits[], \
		 const int radius);
/*
 * Prepare a updated current backend using render_time().
 * animate_clock() will copy frame from the prepared backend.
 */
void prepare_backend(struct app_all *app);
/*
 * Copy a frame from backend texture to frontend texture,
 * the flip transform will be automatically zoom
 * when using SDL_RenderCopy() with a different rect size.
 */
void copy_frame(struct app_all *app, \
		const SDL_Rect target_rect, \
		const int step, \
		const int max_steps);
/*
 * Using prepare_backend() to prepare a backend.
 * Then animate the flip clock.
 * Using copy_frame() and present it.
 */
void animate_clock(struct app_all *app, \
		   int step);
/*
 * Update time and raise an event to call animate_clock().
 */
void update_time(struct app_all *app);
/*
 * Route and handle events.
 */
void route_event(struct app_all *app, \
		 const int timeout);
/*
 * Free some extras which need to calculate the size
 * due to window resizing. You must free them first
 * before reload them.
 */
void free_extra(struct app_all *app);
/*
 * Free and quit an app.
 */
void quit_app(struct app_all *app);
/*
 * Print help message in console.
 */
void print_help(const struct app_all *app);

#endif
