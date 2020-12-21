/**
 * Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.one/)
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "getarg.h"
#include "flipclock.h"

int main(int argc, char *argv[])
{
#ifdef _WIN32
	char OPT_STRING[] = "hwcst:f:p:";
#else
	char OPT_STRING[] = "hwt:f:";
#endif
	int opt = 0;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	if (TTF_Init() < 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
	struct flipclock *app = flipclock_create();
	/* Dealing with arguments. */
	while ((opt = getarg(argc, argv, OPT_STRING)) != -1) {
		switch (opt) {
		case 'w':
			app->properties.full = false;
			break;
		case 't':
			if (atoi(argopt) == 12)
				app->properties.ampm = true;
			break;
		case 'f':
			app->properties.font_path = argopt;
			break;
		case 'h':
			flipclock_print_help(argv[0]);
			exit(EXIT_SUCCESS);
			break;
#ifdef _WIN32
		/**
		 * I have no idea about how to configure it without arguments
		 * in Windows. So just tell user and exit.
		 */
		case 'c':
			MessageBox(NULL,
				   "I am just a UNIX program that "
				   "happens to support Windows, "
				   "and I think registry table is ugly, "
				   "so configuration shoule not be here!",
				   PROGRAM_TITLE, MB_OK);
			exit(EXIT_SUCCESS);
			break;
		case 's':
			/**
			 * One of the most silly requirement I've seen.
			 * But it seems I can use it to handle key press.
			 */
			app->properties.screensaver = true;
			break;
		case 'p':
			app->properties.preview = true;
			app->properties.preview_window = atoi(argopt);
			break;
#endif
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

	flipclock_create_clocks(app);
	for (int i = 0; i < app->clocks_length; ++i) {
		flipclock_refresh(app, i);
		flipclock_open_fonts(app, i);
		flipclock_create_textures(app, i);
	}

	flipclock_run_mainloop(app);

	for (int i = 0; i < app->clocks_length; ++i) {
		flipclock_destroy_textures(app, i);
		flipclock_close_fonts(app, i);
	}
	flipclock_destroy_clocks(app);

	flipclock_destroy(app);
	TTF_Quit();
	SDL_Quit();
	return 0;
}
