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
		// Default width and height.
		int width;
		int height;
		double scale;
		// Varibles about time rect.
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


	bool init_app(struct app_all *app);
	void render_background(struct app_all *app, \
			       SDL_Texture *target_texture, \
			       SDL_Color background_color);
	void render_rounded_box(struct app_all *app, \
				SDL_Rect target_rect, \
				int radius);
	void render_time(struct app_all *app, \
			 SDL_Texture *target_texture, \
			 SDL_Rect target_rect, \
			 TTF_Font *font, \
			 char digits[], \
		 	 int radius);
	void render_frame(struct app_all *app, \
			  SDL_Rect target_rect, \
			  int step, \
			  int max_steps);
	void render_clock(struct app_all *app);
	Uint32 update_time(Uint32 interval, \
			   void *param);
	void quit_app(struct app_all *app);
	void print_help(char *program_name);

#endif
