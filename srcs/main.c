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
#ifdef _WIN32
	/* This is one of the most silly requirement I've seen. */
	bool silly_windows_run_screensaver_with_this_option = false;
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
	struct flipclock *app = flipclock_create();
	/* Dealing with arguments. */
	while ((opt = getarg(argc, argv, OPT_STRING)) != -1) {
		switch (opt) {
		case 'w':
			app->properties.full = false;
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
#ifdef _WIN32
		/*
		 * I have no idea about how to configure it without arguments
		 * in Windows. So just tell user and exit.
		 */
		case 'c':
			MessageBox(
				NULL,
				"Configuration should NOT be here, silly Windows!",
				"FlipClock", MB_OK);
			exit(EXIT_SUCCESS);
			break;
		case 's':
			silly_windows_run_screensaver_with_this_option = true;
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

#ifdef _WIN32
	if (!silly_windows_run_screensaver_with_this_option)
		goto win32_bye;
#endif

	flipclock_create_window(app);
	flipclock_refresh(app);
	flipclock_open_fonts(app);
	flipclock_create_textures(app);

	flipclock_run_mainloop(app);

	flipclock_destroy_textures(app);
	flipclock_close_fonts(app);
	flipclock_destroy_window(app);

#ifdef _WIN32
win32_bye:
#endif
	flipclock_destroy(app);
	TTF_Quit();
	SDL_Quit();
	return 0;
}
