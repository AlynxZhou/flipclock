/*
 * Filename: main.c
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 */
#include "flipclock.h"

int main(int argc, const char *argv[])
{
 	const char FALLBACK_FONT[] = "flipclock.ttf";
	const char OPT_STRING[] = "hwt:f:s:";
	const char TITLE[] = "FlipClock";
	const char VERSION[] = "1.3.3";
	struct app_all flipclock;
	/* Init default content. */
	flipclock.window = NULL;
	flipclock.renderer = NULL;
	flipclock.textures.texture = NULL;
	flipclock.textures.current = NULL;
	flipclock.textures.previous = NULL;
	flipclock.fonts.time = NULL;
	flipclock.fonts.mode = NULL;
	flipclock.colors.font.r = 0xb7;
	flipclock.colors.font.g = 0xb7;
	flipclock.colors.font.b = 0xb7;
	flipclock.colors.font.a = 0xff;
	flipclock.colors.rect.r = 0x17;
	flipclock.colors.rect.g = 0x17;
	flipclock.colors.rect.b = 0x17;
	flipclock.colors.rect.a = 0xff;
	flipclock.colors.black.r = 0x00;
	flipclock.colors.black.g = 0x00;
	flipclock.colors.black.b = 0x00;
	flipclock.colors.black.a = 0xff;
	flipclock.colors.transparent.r = 0x00;
	flipclock.colors.transparent.g = 0x00;
	flipclock.colors.transparent.b = 0x00;
	flipclock.colors.transparent.a = 0x00;
	flipclock.properties.title = TITLE;
	flipclock.properties.version = VERSION;
	flipclock.properties.font_path = NULL;
	flipclock.properties.fallback_font = FALLBACK_FONT;
	flipclock.properties.full = true;
	flipclock.properties.ampm = false;
	flipclock.properties.width = 1024;
	flipclock.properties.height = 768;
	flipclock.properties.scale = 0.0;
	flipclock.properties.program_name = argv[0];
	/* Dealing with argument. */
	int arg;
	while ((arg = getarg(argc, argv, OPT_STRING)) != -1) {
		switch (arg) {
		case 'w':
			flipclock.properties.full = false;
			break;
		case 't':
			if (strcmp(optarg, "12") == 0)
				flipclock.properties.ampm = true;
			break;
		case 'f':
			flipclock.properties.font_path = optarg;
			break;
		case 's':
			sscanf(optarg, "%lf", \
			       &flipclock.properties.scale);
			break;
		case 'h':
			print_help(&flipclock);
			exit(EXIT_SUCCESS);
			break;
		default:
			break;
        	}
    	}
	/* Try to init app. */
	if (!init_app(&flipclock)) {
		quit_app(&flipclock);
		exit(EXIT_FAILURE);
	}
	/* Listen for update. */
	bool quit = false;
	bool wait = false;
	time_t raw_time;
	SDL_Event event;
	// SDL_TimerID timer = SDL_AddTimer(250, update_time, &flipclock);
	while (!quit) {
		raw_time = time(NULL);
		flipclock.times.now = *localtime(&raw_time);
		if (flipclock.times.now.tm_min != \
		    flipclock.times.past.tm_min || \
		    flipclock.times.now.tm_hour != \
		    flipclock.times.past.tm_hour) {
			SDL_Event timer_event;
			timer_event.type = SDL_USEREVENT;
			timer_event.user.code = 0;
			timer_event.user.data1 = NULL;
			timer_event.user.data2 = NULL;
			SDL_PushEvent(&timer_event);
		}
		if (SDL_WaitEventTimeout(&event, 10)) {
			switch (event.type) {
			case SDL_USEREVENT:
				/* Time to update. */
				if (!wait)
					animate_clock(&flipclock, 0);
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
				case SDL_WINDOWEVENT_MINIMIZED:
					wait = true;
					break;
				case SDL_WINDOWEVENT_RESTORED:
					wait = false;
					refresh_content(&flipclock, MAX_STEPS);
					break;
				case SDL_WINDOWEVENT_CLOSE:
					quit = true;
					break;
				default:
					break;
				}
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
				case SDLK_q:
					/* Press `q` or `Esc` to quit. */
					quit = true;
					break;
				case SDLK_t:
					/* Press `t` to toggle type. */
					flipclock.properties.ampm = \
					!flipclock.properties.ampm;
					refresh_content(&flipclock, MAX_STEPS);
					break;
				default:
					break;
				}
				break;
			case SDL_QUIT:
				quit = true;
				break;
			default:
				break;
			}
		}
	}
	// SDL_RemoveTimer(timer);
	quit_app(&flipclock);
	return 0;
}
