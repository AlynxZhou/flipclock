/*
 * Filename: flipclock.h
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 */
#ifndef _FLIPCLOCK_H
#	define _FILPCLOCK_H

#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>
#	include <stdbool.h>
#	include <time.h>

#	include "SDL2/SDL.h"
#	include "SDL2/SDL_ttf.h"
#	include "getarg/getarg.h"

#	define FALLBACK_FONT "flipclock.ttf"
#	define OPT_STRING "hwt:f:s:"
#	define TITLE "FlipClock"
#	define VERSION "1.1.1"
#	define MAX_STEPS 180

	struct app_properties {
		char *program_name;
		char *font_path;
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
	 * Clear texture with given color.
	 */
	void clear_texture(struct app_all *app, \
			   SDL_Texture *target_texture, \
			   SDL_Color background_color);
	/*
	 * Draw a rounded box in given rect.
	 * Using Bresenham's circle algorithm.
	 */
	void draw_rounded_box(struct app_all *app, \
			      SDL_Rect target_rect, \
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
			 SDL_Rect target_rect, \
			 TTF_Font *font, \
			 char digits[], \
			 int radius);
	/*
	 * Copy a frame from backend texture to frontend texture,
	 * the flip transform will be automatically zoom
	 * when using SDL_RenderCopy() with a different rect size.
	 */
	void copy_frame(struct app_all *app, \
			SDL_Rect target_rect, \
			int step, \
			int max_steps);
	/*
	 * Switch the two backend texture,
	 * and the update the current backend when time digits changes.
	 * Using render_time().
	 * Then animate the flip clock.
	 * Using copy_frame() and present it.
	 */
	void animate_clock(struct app_all *app);
	/*
	 * Update time and raise an event to call animate_clock().
	 * Used by a timer. Param is the app pointer.
	 */
	Uint32 update_time(Uint32 interval, \
			   void *param);
	/*
	 * Free and quit an app.
	 */
	void quit_app(struct app_all *app);
	/*
	 * Print help message in console.
	 */
	void print_help(char *program_name);

#endif
