/**
 * Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.one/)
 */
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
		LOG_ERROR("SDL Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	if (TTF_Init() < 0) {
		LOG_ERROR("SDL Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
	struct flipclock *app = flipclock_create();
	/* Android don't need conf and arguments. */
#ifndef __ANDROID__
	flipclock_load_conf(app);
	/* Dealing with arguments which have higher priority. */
	while ((opt = getarg(argc, argv, OPT_STRING)) != -1) {
		switch (opt) {
		case 'w':
			app->properties.full = false;
			break;
		case 't':
			if (atoi(argopt) == 12)
				app->properties.ampm = true;
			else if (atoi(argopt) == 24)
				app->properties.ampm = false;
			break;
		case 'f':
			strncpy(app->properties.font_path, argopt,
				MAX_BUFFER_LENGTH - 1);
			app->properties.font_path[MAX_BUFFER_LENGTH - 1] = '\0';
			if (strlen(app->properties.font_path) ==
			    MAX_BUFFER_LENGTH - 1) {
				LOG_ERROR("font_path too long, "
					  "may fail to load.\n");
			}
			break;
		case 'h':
			flipclock_print_help(app, argv[0]);
			goto exit;
			break;
		case 'v':
			printf(PROJECT_VERSION "\n");
			goto exit;
			break;
#	ifdef _WIN32
		case 'c':
			MessageBox(NULL,
				   "Please read and edit flipclock.conf "
				   "under program directory to configure it!",
				   PROGRAM_TITLE, MB_OK);
			goto exit;
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
#	endif
		case 0:
			LOG_ERROR("%s: Invalid value `%s`.\n", argv[0], argopt);
			break;
		default:
			LOG_ERROR("%s: Invalid option `%c%c`.\n", argv[0],
				  OPT_START, opt);
			break;
		}
	}
#endif

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

exit:
	flipclock_destroy(app);
	TTF_Quit();
	SDL_Quit();
	return 0;
}
