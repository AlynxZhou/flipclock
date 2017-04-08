/*
 * Filename: main.c
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 */
#include "flipclock.h"

// Global SDL varibles.
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

// Time.
struct tm _prevTime;
struct tm *prevTime = &_prevTime;
struct tm _nowTime;
struct tm *nowTime = &_nowTime;
// Some global resource.
const char FALLBACKFONT[] = "flipclock.ttf";
const char OPTSTRING[] = "hwt:f:s:";
const char TITLE[] = "FlipClock";
const char *fontPath = NULL;
const int MAXSTEPS = 180;
bool full = true;
bool ampm = false;
// Default width and height.
int width = 1024;
int height = 768;
double scaleFactor = 0.0;
// Varibles about time rect.
int rectSize = 0;
int wSpace = 0;
int radius = 0;

int main(int argc, const char *argv[])
{
	// Dealing with argument.
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

	// Try to init app.
	if (!appInit(argv[0])) {
		appQuit();
		exit(EXIT_FAILURE);
	}

	// This keeps a safe numbers which will let the clock init.
	prevTime->tm_hour = -25;
	prevTime->tm_min = -25;

	// Listen for update.
	bool quit = false;
	SDL_Event event;
	SDL_TimerID timer = SDL_AddTimer(60, timeUpdater, prevTime);

	while (!quit && SDL_WaitEvent(&event)) {
		switch (event.type) {
			case SDL_USEREVENT:
				// Time to update.
				renderClock();
				break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
					// Use <q> or <Esc> to quit.
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
