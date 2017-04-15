/*
 * Filename: flipclock.c
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 */
#include "flipclock.h"

bool appInit(void)
{
	// Init SDL.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "%s: SDL could not be inited! " \
			"SDL Error: %s\n", programName, SDL_GetError());
		SDL_Quit();
		return false;
	}
	// Calculate numbers.
	if (scaleFactor != 0.0) {
		full = false;
		width *= scaleFactor;
		height *= scaleFactor;
	} else if (full) {
		SDL_DisplayMode displayMode;
		SDL_GetCurrentDisplayMode(0, &displayMode);
		width = displayMode.w;
		height = displayMode.h;
		SDL_ShowCursor(SDL_DISABLE);
	}
	rectSize = width * 0.4;
	wSpace = width * 0.06;
	radius = rectSize / 10;
	hourRect->x = (width - 2 * rectSize - wSpace) / 2;
	hourRect->y = (height - rectSize) / 2;
	hourRect->w = rectSize;
	hourRect->h = rectSize;
	minuteRect->x = hourRect->x + hourRect->w + wSpace;
	minuteRect->y = hourRect->y;
	minuteRect->w = rectSize;
	minuteRect->h = rectSize;
	modeRect->w = rectSize / 4;
	modeRect->h = rectSize / 8;
	modeRect->x = (width - modeRect->w) / 2;
	modeRect->y = (height - rectSize) / 2 + rectSize + \
		      ((height - rectSize) / 2 - modeRect->h) / 2;
	// Create window.
	Window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, \
				  SDL_WINDOWPOS_UNDEFINED, width, height, \
				  (full ? SDL_WINDOW_FULLSCREEN_DESKTOP : \
				  SDL_WINDOW_SHOWN)|
				  SDL_WINDOW_ALLOW_HIGHDPI);
	if (Window == NULL) {
		fprintf(stderr, "%s: Window could not be created! " \
			"SDL Error: %s\n", programName, SDL_GetError());
		return false;
	}
	// Create renderer.
	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | \
				      SDL_RENDERER_TARGETTEXTURE);
	if (Renderer == NULL) {
		fprintf(stderr, "%s: Renderer could not be created! " \
			"SDL Error: %s\n", programName, SDL_GetError());
		return false;
	}
	// Main screen buffer texture.
	Texture = SDL_CreateTexture(Renderer, 0, \
				    SDL_TEXTUREACCESS_TARGET, \
				    width, height);
	if (Texture == NULL) {
		fprintf(stderr, "%s: Texture could not be created! " \
			"SDL Error: %s\n", programName, SDL_GetError());
		return false;
	}
	// Fill with black.
	renderBackGround(Texture, blackColor);
	// Two transparent texture swap for tribuffer.
	currTexture = SDL_CreateTexture(Renderer, 0, \
					SDL_TEXTUREACCESS_TARGET, \
					width, height);
	if (currTexture == NULL) {
		fprintf(stderr, "%s: Texture could not be created! " \
			"SDL Error: %s\n", programName, SDL_GetError());
		return false;
	}
	renderBackGround(currTexture, transparent);
	prevTexture = SDL_CreateTexture(Renderer, 0, \
					SDL_TEXTUREACCESS_TARGET, \
					width, height);
	if (currTexture == NULL) {
		fprintf(stderr, "%s: Texture could not be created! " \
			"SDL Error: %s\n", programName, SDL_GetError());
		return false;
	}
	renderBackGround(prevTexture, transparent);
	// Init SDL_ttf.
	if (TTF_Init() < 0) {
		fprintf(stderr, "%s: SDL_ttf could not initialize! " \
			"SDL_ttf Error: %s\n", programName, TTF_GetError());
		return false;
	}
	// Load custom/fallback font.
	if (fontPath != NULL) {
		timeFont = TTF_OpenFont(fontPath, rectSize);
		modeFont = TTF_OpenFont(fontPath, rectSize / 10);
		if (timeFont == NULL || modeFont == NULL) {
			fprintf(stderr, "%s: Custom font " \
				"could not be opened! " \
				"TTF Error: %s\n", \
				programName, TTF_GetError());
			timeFont = TTF_OpenFont(FALLBACKFONT, rectSize);
			modeFont = TTF_OpenFont(FALLBACKFONT, rectSize / 10);
		}
		if (timeFont == NULL || modeFont == NULL) {
			fprintf(stderr, "%s: Fallback font " \
				"could not be opened! " \
				"TTF Error: %s\n", \
				programName, TTF_GetError());
			return false;
		}
	} else {
		timeFont = TTF_OpenFont(FALLBACKFONT, rectSize);
		modeFont = TTF_OpenFont(FALLBACKFONT, rectSize / 10);
		if (timeFont == NULL || modeFont == NULL) {
			fprintf(stderr, "%s: Fallback font " \
				"could not be opened! " \
				"TTF Error: %s\n", \
				programName,  TTF_GetError());
			return false;
		}
	}
	return true;
}

// Clear texture with color.
void renderBackGround(SDL_Texture *targetTexture, \
		      const SDL_Color *backGroundColor)
{
	SDL_SetRenderTarget(Renderer, targetTexture);
	SDL_SetRenderDrawColor(Renderer, \
			       backGroundColor->r, backGroundColor->g, \
			       backGroundColor->b, backGroundColor->a);
	SDL_RenderClear(Renderer);
}

// Bresenham's circle algorithm.
void renderRoundedBox(SDL_Renderer *Renderer, \
		      const SDL_Rect *targetRect, \
		      int radius)
{
	if (radius <= 1) {
		SDL_RenderFillRect(Renderer, targetRect);
		return;
	}
	if (2 * radius > targetRect->w)
		radius = targetRect->w / 2;
	if (2 * radius > targetRect->h)
		radius = targetRect->h / 2;

	int x = 0, y = radius, d = 3 - 2 * radius;
	SDL_Rect temp;

        while (x <= y) {
		SDL_RenderDrawLine(Renderer, \
				   targetRect->x + radius - x, \
				   targetRect->y + radius - y, \
				   targetRect->x + targetRect->w - \
				   radius + x - 1, \
				   targetRect->y + radius - y);
		SDL_RenderDrawLine(Renderer, \
				   targetRect->x + radius - x, \
				   targetRect->y + targetRect->h - \
				   radius + y, \
				   targetRect->x + targetRect->w - \
				   radius + x - 1, \
				   targetRect->y + targetRect->h - \
				   radius + y);
		SDL_RenderDrawLine(Renderer, \
				   targetRect->x + radius - y, \
				   targetRect->y + radius - x, \
				   targetRect->x + targetRect->w - \
				   radius + y - 1, \
				   targetRect->y + radius - x);
		SDL_RenderDrawLine(Renderer, \
				   targetRect->x + radius - y, \
				   targetRect->y + targetRect->h - \
				   radius + x, \
				   targetRect->x + targetRect->w - \
				   radius + y - 1, \
				   targetRect->y + targetRect->h - \
				   radius + x);
        	if (d < 0) {
			d = d + 4 * x + 6;
        	} else {
			d = d + 4 * (x - y) + 10;
        		y--;
        	}
        	x++;
        }
	temp.x = targetRect->x;
	temp.y = targetRect->y + radius;
	temp.w = targetRect->w;
	temp.h = targetRect->h - 2 * radius;
	SDL_RenderFillRect(Renderer, &temp);
}

// Render time background.
void renderTimeRect(SDL_Texture *targetTexture, \
		    const SDL_Rect *targetRect, \
		    const int radius)
{
	// Use SDL2_gfx to render a roundedBox to
	// Renderer whose target is bgTexture.
	SDL_SetRenderTarget(Renderer, targetTexture);
	SDL_SetRenderDrawColor(Renderer, rectColor->r, rectColor->g, \
			       rectColor->b, rectColor->a);
	renderRoundedBox(Renderer, targetRect, radius);
}

// Render time text to backend buffer.
void renderTimeText(SDL_Texture *targetTexture, \
		    const SDL_Rect *targetRect, \
		    TTF_Font* font, \
		    const char digits[], \
		    const int radius, \
		    const SDL_Color *fontColor)
{
	SDL_Surface *textSurface = NULL;
	SDL_Texture *textTexture = NULL;
	SDL_Rect digitRects;

	if (strlen(digits) != 2) {
		fprintf(stderr, "%s: Wrong time digits!", \
			programName);
			return;
	}

	renderTimeRect(targetTexture, targetRect, radius);
	for (int i = 0; i < 2; i++) {
		textSurface = TTF_RenderGlyph_Blended(font, digits[i], \
						      *fontColor);
		if (textSurface == NULL) {
			fprintf(stderr, "%s: Unable to renderer " \
				"char into surface! " \
				"SDL Error: %s\n", \
				programName, SDL_GetError());
			SDL_FreeSurface(textSurface);
			return;
		}
		textTexture = SDL_CreateTextureFromSurface(Renderer, \
							   textSurface);
		if (textTexture == NULL) {
			fprintf(stderr, "%s: Unable to renderer " \
				"surface into texture! " \
				"SDL Error: %s\n", \
				programName, SDL_GetError());
			SDL_DestroyTexture(textTexture);
			return;
		}
		digitRects.x = targetRect->x + targetRect->w / 2 * i + \
			       (targetRect->w / 2 - textSurface->w) / 2;
		digitRects.y = targetRect->y + (targetRect->h - \
			       textSurface->h) / 2;
		digitRects.w = textSurface->w;
		digitRects.h = textSurface->h;
		SDL_FreeSurface(textSurface);
		SDL_SetRenderTarget(Renderer, targetTexture);
		SDL_RenderCopy(Renderer, textTexture, NULL, &digitRects);
		SDL_DestroyTexture(textTexture);
	}
}

// Move and zoom time from back buffer to front main buffer.
// This will make a part of a frame.
void renderTime(const SDL_Rect *targetRect, \
		const int step, \
		const int maxSteps)
{
	SDL_Rect halfSrcRect, halfDstRect, dividerRect;
	double scale;
	// Draw the upper current digit and render it.
	// No need to render the previous lower digit.
	// For it will remain.
	halfSrcRect.x = targetRect->x;
	halfSrcRect.y = targetRect->y;
	halfSrcRect.w = targetRect->w;
	halfSrcRect.h = targetRect->h / 2;
	halfDstRect.x = targetRect->x;
	halfDstRect.y = targetRect->y;
	halfDstRect.w = targetRect->w;
	halfDstRect.h = targetRect->h / 2;
	SDL_SetRenderTarget(Renderer, Texture);
	SDL_RenderCopy(Renderer, currTexture, &halfSrcRect, &halfDstRect);

	// Calculate the scale factor and the color change.
	int halfSteps = maxSteps / 2;
	bool upperhalf = step <= halfSteps;
	scale = upperhalf ? 1.0 - (1.0 * step) / halfSteps : \
		((1.0 * step) - halfSteps) / halfSteps;

	// Draw the flip. upper half is prev and lower half is current.
	// Just custom the destination Rect, the Renderer will zoom
	// automatically.
	halfSrcRect.x = targetRect->x;
	halfSrcRect.y = targetRect->y + (upperhalf ? 0 : targetRect->h / 2);
	halfSrcRect.w = targetRect->w;
	halfSrcRect.h = targetRect->h / 2;
	halfDstRect.x = targetRect->x;
	halfDstRect.y = targetRect->y + (upperhalf ? (targetRect->h * \
			(1 - scale) / 2) : targetRect->h / 2);
	halfDstRect.w = targetRect->w;
	halfDstRect.h = targetRect->h * scale / 2;
	SDL_SetRenderTarget(Renderer, Texture);
	SDL_RenderCopy(Renderer, upperhalf ? prevTexture : currTexture, \
		       &halfSrcRect, &halfDstRect);

	// render divider
	dividerRect.h = targetRect->h == modeRect->h ? \
			targetRect->h / 40 : targetRect->h / 100;
	dividerRect.w = targetRect->w;
	dividerRect.x = targetRect->x;
	dividerRect.y = targetRect->y + (targetRect->h - dividerRect.h) / 2;
	SDL_SetRenderDrawColor(Renderer, transparent->r, transparent->g, \
			       transparent->b, transparent->a);
	SDL_SetRenderTarget(Renderer, Texture);
	SDL_RenderFillRect(Renderer, &dividerRect);
	SDL_SetRenderTarget(Renderer, NULL);
}

// Find which part of frame should be rendered and render them.
// And render them to main texture then display on screen.
void renderFrame(const int step, \
	         const int maxSteps)
{
	if (ampm && ((nowTime->tm_hour / 12) != (prevTime->tm_hour / 12)))
		renderTime(modeRect, step, maxSteps);
	if (nowTime->tm_hour != prevTime->tm_hour)
		renderTime(hourRect, step, maxSteps);
	if (nowTime->tm_min != prevTime->tm_min)
		renderTime(minuteRect, step, maxSteps);
	SDL_SetRenderTarget(Renderer, NULL);
	SDL_RenderCopy(Renderer, Texture, NULL, NULL);
	SDL_RenderPresent(Renderer);
}

// Calculate time and which frame should be render.
void renderClock(void)
{
	const int DURATION = MAXSTEPS;
	int step;
	bool done = false;
	time_t rawTime;
	rawTime = time(NULL);
	nowTime = localtime(&rawTime);
	char nowDigits[3];
	int mRadius = modeRect->w / 10;

	if (prevTime->tm_hour != nowTime->tm_hour || \
	    prevTime->tm_min != nowTime->tm_min) {
		SDL_Texture *swap = currTexture;
		currTexture = prevTexture;
		prevTexture = swap;
	}
	if (ampm && ((nowTime->tm_hour / 12) != (prevTime->tm_hour / 12))) {
		snprintf(nowDigits, sizeof(nowDigits), "%cM", \
			 nowTime->tm_hour / 12 ? 'P' : 'A');
		renderTimeText(currTexture, modeRect, modeFont, \
			       nowDigits, mRadius, fontColor);
	}
	if (nowTime->tm_hour != prevTime->tm_hour) {
		ampm? strftime(nowDigits, sizeof(nowDigits), "%I", nowTime) : \
		      strftime(nowDigits, sizeof(nowDigits), "%H", nowTime);
		renderTimeText(currTexture, hourRect, timeFont, \
			       nowDigits, radius, fontColor);
	}
	if (nowTime->tm_min != prevTime->tm_min) {
		strftime(nowDigits, sizeof(nowDigits), "%M", nowTime);
		renderTimeText(currTexture, minuteRect, timeFont, \
				nowDigits, radius, fontColor);
	}

	int startTick = SDL_GetTicks();
	int endTick = startTick + DURATION;
	int currentTick;

	while (!done) {
		currentTick = SDL_GetTicks();
		if (currentTick >= endTick) {
			// Align.
			currentTick = endTick;
			done = true;
		}
		step = MAXSTEPS * (currentTick - startTick) / \
		       (endTick - startTick);
		renderFrame(step, MAXSTEPS);
	}
	*prevTime = *nowTime;
}

// Raise events for time update.
Uint32 timeUpdater(Uint32 interval, \
		   void *param)
{
	SDL_Event timerEvent;
	time_t rawTime;
	struct tm *nowTime;
	struct tm *prevTime = (struct tm *)param;

	rawTime = time(NULL);
	nowTime = localtime(&rawTime);

	if(nowTime->tm_min != prevTime->tm_min) {
		timerEvent.type = SDL_USEREVENT;
		timerEvent.user.code = 0;
		timerEvent.user.data1 = NULL;
		timerEvent.user.data2 = NULL;
		SDL_PushEvent(&timerEvent);
		interval = 1000 * (60 - nowTime->tm_sec) - 250;
	} else {
		interval = 250;
	}

	return interval;
}

void appQuit(void)
{
	TTF_CloseFont(timeFont);
	TTF_CloseFont(modeFont);
	TTF_Quit();
	SDL_DestroyTexture(prevTexture);
	SDL_DestroyTexture(currTexture);
	SDL_DestroyTexture(Texture);
	SDL_DestroyRenderer(Renderer);
	SDL_ShowCursor(SDL_ENABLE);
	SDL_DestroyWindow(Window);
	SDL_Quit();
}

void printHelp(void)
{
	printf("A simple flip clock screensaver using SDL2.\n");
	printf("Written by AlynxZhou. Version %s.\n", VERSION);
	printf("Usage: %s [OPTION...] <value>\n", programName);
	printf("Options:\n");
	printf("\t-h\t\tDisplay this help.\n");
	printf("\t-w\t\tRun in window.\n");
	printf("\t-t <12|24>\tToggle 12-hour clock format (AM/PM) " \
	       "or 24-hour clock format.\n");
	printf("\t-f <font>\tCustom font.\n");
	printf("\t-s <factor>\tCustom resolution with a scale factor.\n");
	printf("Press `q` or `Esc` to quit.\n");
}
