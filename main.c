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
	/* Fill default content. */
	fill_defaults(&flipclock);
	flipclock.properties.title = TITLE;
	flipclock.properties.version = VERSION;
	flipclock.properties.fallback_font = FALLBACK_FONT;
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
	SDL_Event event;
	while (!quit) {
		update_time(&flipclock);
		if (SDL_WaitEventTimeout(&event, 100)) {
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
	quit_app(&flipclock);
	return 0;
}
