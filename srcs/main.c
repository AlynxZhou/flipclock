/*
 * Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.moe/)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getarg.h"
#include "flipclock.h"

int main(int argc, char *argv[])
{
	const char OPT_STRING[] = "hwt:f:s:";
	int opt = 0;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	if (TTF_Init() < 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	struct flipclock *app = flipclock_create();
	flipclock_set_fullscreen(app, true);
	/* Dealing with arguments. */
	while ((opt = getarg(argc, argv, OPT_STRING)) != -1) {
		switch (opt) {
		case 'w':
			flipclock_set_fullscreen(app, false);
			break;
		case 't':
			if (strcmp(argopt, "24") == 0)
				app->properties.ampm = false;
			break;
		case 'f':
			app->properties.font_path = argopt;
			break;
		case 'h':
			flipclock_print_help(app, argv[0]);
			exit(EXIT_SUCCESS);
			break;
		case 0:
			fprintf(stderr, "%s: Invalid value `%s`.\n", argv[0],
				argopt);
			exit(EXIT_FAILURE);
			break;
		default:
			fprintf(stderr, "%s: Invalid option `%c%c`.\n", argv[0],
				OPT_START, opt);
			exit(EXIT_FAILURE);
			break;
		}
	}

	flipclock_refresh(app);
	flipclock_open_fonts(app);
	flipclock_create_textures(app);

	flipclock_run_mainloop(app);

	flipclock_destroy_textures(app);
	flipclock_close_fonts(app);
	flipclock_destroy(app);

	TTF_Quit();
	SDL_Quit();
	return 0;
}
