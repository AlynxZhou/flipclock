/*
 * Filename: flipclock.c
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 */
#include "flipclock.h"

#define TITLE "FlipClock"
#define WIDTH 1024
#define HEIGHT 768
#define MAXSTEPS 180
#define OPTSTRING "hwt:f:s:"
#define FALLBACKFONT "numbers.ttf"

SDL_Window *Window = NULL;
SDL_Renderer *Renderer = NULL;
SDL_Texture *Texture = NULL;
SDL_Texture *currTexture = NULL;
SDL_Texture *prevTexture = NULL;
TTF_Font *timeFont = NULL;
TTF_Font *modeFont = NULL;
const SDL_Color FONTCOLOR = {0xb7, 0xb7, 0xb7, 0xff};
const SDL_Color RECTCOLOR = {0x17, 0x17, 0x17, 0xff};
const SDL_Color BLACKCOLOR = {0x00, 0x00, 0x00, 0xff};
const SDL_Color TRANSPARENT = {0x00, 0x00, 0x00, 0x00};
const SDL_Color *fontColor = &FONTCOLOR;
const SDL_Color *rectColor = &RECTCOLOR;
const SDL_Color *blackColor = &BLACKCOLOR;
const SDL_Color *transparent = &TRANSPARENT;
SDL_Rect hourRect;
SDL_Rect minuteRect;
SDL_Rect modeRect;

struct tm _prevTime;
struct tm *prevTime = &_prevTime;

bool ampm = false;
bool full = true;

const char *fontPath = FALLBACKFONT;

int width = WIDTH;
int height = HEIGHT;
double scaleFactor = 0.0;
int rectSize = 0;
int wSpace = 0;
int radius = 0;

int main(int argc, const char *argv[])
{
	int arg;
	while ((arg = getArg(argc, argv, OPTSTRING)) != -1) {
		switch(arg) {
			case 'w':
				full = false;
				break;
			case 't':
				if (strcmp(optarg, "12") == 0)
					ampm = true;
				break;
			case 'f':
				fontPath = optarg;
				break;
			case 's':
				sscanf(optarg, "%lf", &scaleFactor);
				break;
			case 'h':
				printHelp(argv[0]);
				exit(EXIT_SUCCESS);
				break;
			default:
        			break;
        	}
    	}

	if (!appInit(argv[0])) {
		appQuit();
		exit(EXIT_FAILURE);
	}

	prevTime->tm_hour = -25;
	prevTime->tm_min = -25;

	renderClock(MAXSTEPS - 1, MAXSTEPS);

	bool quit = false;
	SDL_Event event;
	SDL_TimerID timer = SDL_AddTimer(60, timeUpdater, prevTime);

	while (!quit && SDL_WaitEvent(&event)) {
		switch (event.type) {
			case SDL_USEREVENT:
				renderAnimate();
				break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
					case SDLK_ESCAPE:
					case SDLK_q:
						quit = true;
						break;
					default:
						break;
				}
				break;
			case SDL_QUIT:
				quit = true;
				break;
		}
	}

	SDL_RemoveTimer(timer);

	appQuit();

	return 0;
}

bool appInit(const char programName[])
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "%s: Unable to init SDL: %s\n", programName, SDL_GetError());
		SDL_Quit();
		return false;
	}

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
	hourRect.x = (width - 2 * rectSize - wSpace) / 2;
	hourRect.y = (height - rectSize) / 2;
	hourRect.w = rectSize;
	hourRect.h = rectSize;
	minuteRect.x = hourRect.x + hourRect.w + wSpace;
	minuteRect.y = hourRect.y;
	minuteRect.w = rectSize;
	minuteRect.h = rectSize;
	modeRect.w = rectSize / 5;
	modeRect.h = rectSize / 10;
	modeRect.x = (width - modeRect.w) / 2;
	modeRect.y = (height - rectSize) / 2 + rectSize + ((height - rectSize) / 2 - modeRect.h) / 2;
	Window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, (full? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_SHOWN));
	if (Window == NULL) {
		fprintf(stderr, "%s: Window could not be created! SDL Error: %s\n", programName, SDL_GetError());
		return false;
	}
	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE);
	if (Renderer == NULL) {
		fprintf(stderr, "%s: Renderer could not be created! SDL Error: %s\n", programName, SDL_GetError());
		return false;
	}
	Texture = SDL_CreateTexture(Renderer, 0, SDL_TEXTUREACCESS_TARGET, width, height);
	if (Texture == NULL) {
		fprintf(stderr, "%s: Texture could not be created! SDL Error: %s\n", programName, SDL_GetError());
		return false;
	}
	renderBackGround(Texture, blackColor);
	currTexture = SDL_CreateTexture(Renderer, 0, SDL_TEXTUREACCESS_TARGET, width, height);
	if (currTexture == NULL) {
		fprintf(stderr, "%s: Texture could not be created! SDL Error: %s\n", programName, SDL_GetError());
		return false;
	}
	renderBackGround(currTexture, transparent);
	if (TTF_Init() < 0) {
		fprintf(stderr, "%s: SDL_ttf could not initialize! SDL_ttf Error: %s\n", programName, TTF_GetError());
		return false;
	}
	timeFont = TTF_OpenFont(fontPath, rectSize);
	modeFont = TTF_OpenFont(fontPath, rectSize / 10);
	if ((timeFont == NULL || modeFont == NULL) && strcmp(fontPath, FALLBACKFONT) != 0) {
		fprintf(stderr, "%s: Custom font could not be opened! TTF Error: %s\n", programName, TTF_GetError());
		timeFont = TTF_OpenFont(FALLBACKFONT, rectSize);
		modeFont = TTF_OpenFont(FALLBACKFONT, rectSize / 10);
	}
	if ((timeFont == NULL || modeFont == NULL) && strcmp(fontPath, FALLBACKFONT) == 0) {
		fprintf(stderr, "%s: Fallback font could not be opened! TTF Error: %s\n", programName,  TTF_GetError());
		return false;
	}
	return true;
}

void renderBackGround(SDL_Texture *targetTexture,
		      const SDL_Color *backGroundColor)
{
	SDL_SetRenderTarget(Renderer, targetTexture);
	SDL_SetRenderDrawColor(Renderer, backGroundColor->r, backGroundColor->g, backGroundColor->b, backGroundColor->a);
	SDL_RenderClear(Renderer);
}

void renderTimeBackGround(const SDL_Rect *targetRect,
			  const int radius)
{
	// Use SDL2_gfx to render a roundedBox to
	// Renderer whose target is bgTexture.
	roundedBoxRGBA(Renderer, targetRect->x, targetRect->y, targetRect->x + targetRect->w, targetRect->y + targetRect->h, radius, rectColor->r, rectColor->g, rectColor->b, rectColor->a);
}

void renderClockDigits(SDL_Texture *targetTexture,
		       const SDL_Rect *targetRect,
		       TTF_Font* font,
		       const char digits[],
		       const int radius,
	     	       const SDL_Color tempColor)
{
	SDL_Surface *textSurface = NULL;
	SDL_Texture *textTexture = NULL;
	SDL_Rect digitRects;

	SDL_SetRenderTarget(Renderer, targetTexture);
	renderTimeBackGround(targetRect, radius);
	// if (strlen(digits) == 2) {
		// first digit
	textSurface = TTF_RenderGlyph_Blended(font, digits[0], tempColor);
	textTexture = SDL_CreateTextureFromSurface(Renderer, textSurface);
	digitRects.x = targetRect->x + (targetRect->w / 2 - textSurface->w) / 2;
	digitRects.y = targetRect->y + (targetRect->h - textSurface->h) / 2;
	digitRects.w = textSurface->w;
	digitRects.h = textSurface->h;
	SDL_FreeSurface(textSurface);
	SDL_RenderCopy(Renderer, textTexture, NULL, &digitRects);
	textSurface = TTF_RenderGlyph_Blended(font, digits[1], tempColor);
	textTexture = SDL_CreateTextureFromSurface(Renderer, textSurface);
	digitRects.x = targetRect->x + targetRect->w / 2 + (targetRect->w / 2 - textSurface->w) / 2;
	digitRects.y = targetRect->y + (targetRect->h - textSurface->h) / 2;
	digitRects.w = textSurface->w;
	digitRects.h = textSurface->h;
	SDL_FreeSurface(textSurface);
	SDL_RenderCopy(Renderer, textTexture, NULL, &digitRects);
	SDL_DestroyTexture(textTexture);
	SDL_SetRenderTarget(Renderer, Texture);
}

void renderTime(SDL_Texture *targetTexture,
	      const SDL_Rect *targetRect,
	      TTF_Font *font,
	      const char nowDigits[],
	      const char prevDigits[],
	      const int radius,
	      const int step,
	      const int maxSteps)
{

	SDL_SetRenderTarget(Renderer, Texture);
	SDL_Rect halfSrcRect, halfDstRect, dividerRect;
	SDL_Color tempColor = *fontColor;
	double scale;
	// Draw the upper current digit and render it.
	// No nedd to render the previous lower digit.
	// For it will remain.
	halfSrcRect.x = targetRect->x;
	halfSrcRect.y = targetRect->y;
	halfSrcRect.w = targetRect->w;
	halfSrcRect.h = targetRect->h / 2;
	halfDstRect.x = targetRect->x;
	halfDstRect.y = targetRect->y;
	halfDstRect.w = targetRect->w;
	halfDstRect.h = targetRect->h / 2;
	renderClockDigits(targetTexture, targetRect, font, nowDigits, radius, tempColor);
	SDL_SetRenderTarget(Renderer, Texture);
	SDL_RenderCopy(Renderer, targetTexture, &halfSrcRect, &halfDstRect);

	// Calculate the scale factor and the color change.
	int halfSteps = maxSteps / 2;
	bool upperhalf = step < halfSteps;
	if(upperhalf) {
		scale = 1.0 - (1.0 * step) / (halfSteps - 1);
		tempColor.r = tempColor.g = tempColor.b = 0xb7 - 0xb7 * (1.0 * step) / (halfSteps - 1);
	} else {
		scale = ((1.0 * step) - halfSteps + 1) / halfSteps;
		tempColor.r = tempColor.g = tempColor.b = 0xb7 * ((1.0 * step) - halfSteps + 1) / halfSteps;
	}

	// Draw the flip. upper half is prev and lower half is current.
	// Just custom the destination Rect, the Renderer will zoom
	// automatically.
	halfSrcRect.x = targetRect->x;
	halfSrcRect.y = targetRect->y + (upperhalf? 0 : targetRect->h / 2);
	halfSrcRect.w = targetRect->w;
	halfSrcRect.h = targetRect->h / 2;
	halfDstRect.x = targetRect->x;
	halfDstRect.y = targetRect->y + (upperhalf? (targetRect->h * (1 - scale) / 2) : targetRect->h / 2);
	halfDstRect.w = targetRect->w;
	halfDstRect.h = targetRect->h * scale / 2;
	renderClockDigits(targetTexture, targetRect, font, upperhalf? prevDigits : nowDigits, radius, tempColor);
	SDL_SetRenderTarget(Renderer, Texture);
	SDL_RenderCopy(Renderer, targetTexture, &halfSrcRect, &halfDstRect);

	// render divider
	dividerRect.h = targetRect->h / 100;
	dividerRect.w = targetRect->w;
	dividerRect.x = targetRect->x;
	dividerRect.y = targetRect->y + (targetRect->h - dividerRect.h) / 2;
	SDL_SetRenderDrawColor(Renderer, transparent->r, transparent->g, transparent->b, transparent->a);
	SDL_SetRenderTarget(Renderer, Texture);
	SDL_RenderFillRect(Renderer, &dividerRect);
	SDL_SetRenderTarget(Renderer, NULL);
}

void renderClock(const int step,
	         const int maxSteps)
{
	struct tm *nowTime;
	time_t rawTime;
	rawTime = time(NULL);
	nowTime = localtime(&rawTime);
	char nowDigits[3];
	char prevDigits[3];
	int mRadius = modeRect.w / 10;
	if (ampm && ((nowTime->tm_hour / 12) != (prevTime->tm_hour / 12))) {
		snprintf(nowDigits, sizeof(nowDigits), "%cM", nowTime->tm_hour / 12? 'P' : 'A');
		snprintf(prevDigits, sizeof(prevDigits), "%cM", prevTime->tm_hour / 12? 'P' : 'A');
		renderTime(currTexture, &modeRect, modeFont, nowDigits, prevDigits, mRadius, step, maxSteps);
	}
	if (nowTime->tm_hour != prevTime->tm_hour) {
		ampm? strftime(nowDigits, sizeof(nowDigits), "%I", nowTime) : strftime(nowDigits, sizeof(nowDigits), "%H", nowTime);
		ampm? strftime(prevDigits, sizeof(prevDigits), "%I", prevTime) : strftime(prevDigits, sizeof(prevDigits), "%H", prevTime);
		renderTime(currTexture, &hourRect, timeFont, nowDigits, prevDigits, radius, step, maxSteps);
	}
	if (nowTime->tm_min != prevTime->tm_min) {
		strftime(nowDigits, sizeof(nowDigits), "%M", nowTime);
		strftime(prevDigits, sizeof(prevDigits), "%M", prevTime);
		renderTime(currTexture, &minuteRect, timeFont, nowDigits, prevDigits, radius, step, maxSteps);

	}
	SDL_SetRenderTarget(Renderer, NULL);
	SDL_RenderCopy(Renderer, Texture, NULL, NULL);
	SDL_RenderPresent(Renderer);
	if (step == maxSteps - 1)
		*prevTime = *nowTime;
}

void renderAnimate(void)
{
	const int DURATION = 180;
	int startTick = SDL_GetTicks();
	int endTick = startTick + DURATION;
	int currentTick;
	int step;
	bool done = false;

	while(!done) {
		currentTick = SDL_GetTicks();
		if(currentTick >= endTick) {
			done = true;
			currentTick = endTick;
		}
		step = (MAXSTEPS - 1) * (currentTick - startTick) / (endTick - startTick);
		printf("%d\n", prevTime->tm_min);
		renderClock(step, MAXSTEPS);
	}
}

Uint32 timeUpdater(Uint32 interval,
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

void printHelp(const char programName[])
{
	printf("A simple flip clock using SDL2 by AlynxZhou. Version 1.0.\n");
	printf("Usage: %s [OPTION...] <value>\n", programName);
	printf("Options:\n");
	printf("\t-h\t\tDisplay this help.\n");
	printf("\t-w\t\tRun in window.\n");
	printf("\t-t <12|24>\tToggle 12-hour clock format (AM/PM) or 24-hour clock format.\n");
	printf("\t-f <font>\tCustom font.\n");
	// printf("  -s <scaleFactor>\t\tCustom resolution with a scale factor, e.g. 1024x768.\n");
}
