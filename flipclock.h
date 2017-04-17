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
#	define FALLBACK_FONT "flipclock.ttf";
#	define OPTSTRING "hwt:f:s:"
#	define TITLE "FlipClock"
#	define VERSION "1.1.1"
#	define MAXSTEPS 180

	typedef struct app_colors {
		SDL_Color font = {0xb7, 0xb7, 0xb7, 0xff};
		SDL_Color rect = {0x17, 0x17, 0x17, 0xff};
		SDL_Color black = {0x00, 0x00, 0x00, 0xff};
		SDL_Color transparent = {0x00, 0x00, 0x00, 0x00};
	} Colors;
	typedef struct app_rects {
		SDL_Rect hour;
		SDL_Rect minute;
		SDL_Rect mode;
	} Rects;
	typedef struct app_fonts {
		TTF_Font *time = NULL;
		TTF_Font *mode = NULL;
	} Fonts;
	typedef struct app_times {
		struct tm past;
		struct tm now;
	} Times;
	typedef struct app_properties {
		const char *font_path = NULL;
		const char *program_name = NULL;
		bool full = true;
		bool ampm = false;
		// Default width and height.
		int width = 1024;
		int height = 768;
		double scale = 0.0;
		// Varibles about time rect.
		int rectSize = 0;
		int wSpace = 0;
		int radius = 0;
	} Properties;
	typedef struct app_textures {
		SDL_Texture *texture = NULL;
		SDL_Texture *current = NULL;
		SDL_Texture *previous = NULL;
	} Textures;
	typedef struct app {
		SDL_Window *window = NULL;
		SDL_Renderer *renderer = NULL;
		struct app_textures textures;
		struct app_rects rects;
		struct app_fonts fonts;
		struct app_times times;
		struct app_colors colors;
		struct app_properties properties;
	} App;


	bool appInit(void);
	void renderBackGround(SDL_Texture *targetTexture, \
			      const SDL_Color *backGroundColor);
	void renderRoundedBox(SDL_Renderer *Renderer, \
			      const SDL_Rect *rect, \
			      int radius);
	void renderTimeRect(SDL_Texture *targetTexture, \
			    const SDL_Rect *targetRect, \
			    const int radius);
	void renderTimeText(SDL_Texture *targetTexture, \
			    const SDL_Rect *targetRect, \
			    TTF_Font* font, \
			    const char digits[], \
			    const int radius, \
			    const SDL_Color *fontColor);
	void renderTime(const SDL_Rect *targetRect, \
			const int step, \
			const int maxSteps);
	void renderFrame(const int step, \
		         const int maxSteps);
	void renderClock(void);
	Uint32 timeUpdater(Uint32 interval, \
			   void *param);
	void appQuit(void);
	void printHelp(void);

#endif
