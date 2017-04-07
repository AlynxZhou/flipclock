#ifndef _FLIPCLOCK_H
#	define _FILPCLOCK_H

#	include <stdio.h>
#	include <stdlib.h>
#	include <stdbool.h>
#	include <math.h>
#	include <time.h>

#	include "SDL2/SDL.h"
#	include "SDL2/SDL_ttf.h"
#	include "SDL2/SDL2_gfxPrimitives.h"
#	include "getarg/getarg.h"

	bool appInit(const char programName[]);
	void renderBackGround(SDL_Texture *targetTexture,
			      const SDL_Color *backGroundColor);
	void renderTimeBackGround(const SDL_Rect *targetRect,
				const int radius);
	void renderClockDigits(SDL_Texture *targetTexture,
			     const SDL_Rect *targetRect,
			     TTF_Font* font,
			     const char digits[],
			     const int radius,
		     	     const SDL_Color tempColor);
	void renderTime(SDL_Texture *targetTexture,
		      const SDL_Rect *targetRect,
		      TTF_Font *font,
		      const char nowDigits[],
		      const char prevDigits[],
		      const int radius,
		      const int step,
		      const int maxSteps);
	void renderClock(const int step,
		       const int maxSteps);
	void renderAnimate(void);
	Uint32 timeUpdater(Uint32 interval,
			   void *param);
	void appQuit(void);
	void printHelp(const char programName[]);

#endif
