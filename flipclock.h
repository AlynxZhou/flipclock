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

	extern SDL_Window *Window;
	extern SDL_Renderer *Renderer;
	extern SDL_Texture *Texture;
	extern SDL_Texture *currTexture;
	extern SDL_Texture *prevTexture;
	extern TTF_Font *timeFont;
	extern TTF_Font *modeFont;
	extern const SDL_Color *fontColor;
	extern const SDL_Color *rectColor;
	extern const SDL_Color *blackColor;
	extern const SDL_Color *transparent;
	extern SDL_Rect hourRect;
	extern SDL_Rect minuteRect;
	extern SDL_Rect modeRect;
	extern struct tm *prevTime;
	extern struct tm *nowTime;
	extern bool ampm;
	extern bool full;
	extern const char FALLBACKFONT[];
	extern const char OPTSTRING[];
	extern const char TITLE[];
	extern const char *fontPath;
	extern const int MAXSTEPS;
	extern int width;
	extern int height;
	extern double scaleFactor;
	extern int rectSize;
	extern int wSpace;
	extern int radius;

	bool appInit(const char programName[]);
	void renderBackGround(SDL_Texture *targetTexture,
			      const SDL_Color *backGroundColor);
	void renderRoundedBox(SDL_Renderer *Renderer,
			      const SDL_Rect *rect,
			      int radius);
	void renderTimeRect(SDL_Texture *targetTexture,
			    const SDL_Rect *targetRect,
			    const int radius);
	void renderTimeText(SDL_Texture *targetTexture,
			    const SDL_Rect *targetRect,
			    TTF_Font* font,
			    const char digits[],
			    const int radius,
			    const SDL_Color *fontColor);
	void renderTime(const SDL_Rect *targetRect,
			const int step,
			const int maxSteps);
	void renderFrame(const int step,
		         const int maxSteps);
	void renderClock(void);
	Uint32 timeUpdater(Uint32 interval,
			   void *param);
	void appQuit(void);
	void printHelp(const char programName[]);

#endif
