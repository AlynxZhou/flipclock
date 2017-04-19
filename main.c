/*
 * Filename: main.c
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 */
#include "flipclock.h"

int main(int argc, char *argv[])
{
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
	flipclock.properties.font_path = NULL;
	flipclock.properties.full = true;
	flipclock.properties.ampm = false;
	flipclock.properties.width = 1024;
	flipclock.properties.height = 768;
	flipclock.properties.scale = 0.0;
	flipclock.properties.program_name = argv[0];
	/* Dealing with argument. */
	int arg;
	while ((arg = get_arg(argc, argv, OPT_STRING)) != -1) {
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
			print_help(argv[0]);
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
	/* This keeps safe numbers which will let the clock init. */
	flipclock.times.past.tm_hour = -25;
	flipclock.times.past.tm_min = -25;
	/* Listen for update. */
	bool quit = false;
	SDL_Event event;
	SDL_TimerID timer = SDL_AddTimer(60, update_time, &flipclock);
	while (!quit && SDL_WaitEvent(&event)) {
		switch (event.type) {
		case SDL_USEREVENT:
			/* Time to update. */
			animate_clock(&flipclock);
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			/* Press `q` or `Esc` to quit. */
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
	quit_app(&flipclock);
	return 0;
}
