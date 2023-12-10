/**
 * Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.one/)
 */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "getarg.h"
#include "flipclock.h"

int main(int argc, char *argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		LOG_ERROR("%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	if (TTF_Init() < 0) {
		LOG_ERROR("%s\n", TTF_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
	struct flipclock *app = flipclock_create();
#if !defined(__ANDROID__)
	// Android don't need conf and arguments.
	flipclock_load_conf(app);
#	if defined(__DEBUG__)
	for (int i = 0; i < argc; ++i)
		LOG_DEBUG("argv[%d]: %s\n", i, argv[i]);
#	endif
#	if defined(_WIN32)
	char OPT_STRING[] = "hvscp:3wt:f:";
#	else
	char OPT_STRING[] = "hv3wt:f:";
#	endif
	int opt = 0;
	bool exit_after_argument = false;
	while ((opt = getarg(argc, argv, OPT_STRING)) != -1) {
		switch (opt) {
		case 'h':
			flipclock_print_help(app, argv[0]);
			exit_after_argument = true;
			break;
		case 'v':
			printf(PROJECT_VERSION "\n");
			exit_after_argument = true;
			break;
#	if defined(_WIN32)
		// See <https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ms686421(v=vs.85)#concepts>.
		case 's':
			/**
			 * One of the most silly requirement I've seen.
			 * But it seems I can use it to handle key press.
			 */
			app->screensaver = true;
			/**
			 * Even if user set windowed mode in configuration file,
			 * screensaver still needs to be fullscreen.
			 */
			app->full = true;
			break;
		case 'c':
			MessageBox(NULL,
				   "Please read and edit flipclock.conf "
				   "under program directory to configure it!",
				   PROGRAM_TITLE, MB_OK);
			exit_after_argument = true;
			break;
		case 'p':
			app->preview = true;
			if (argopt == NULL) {
				LOG_ERROR("Missing value for option `%c%c`\n",
					  OPT_START, opt);
				exit_after_argument = true;
				break;
			}
			/**
			 * See <https://docs.microsoft.com/en-us/windows/win32/winprog/windows-data-types>.
			 * typedef void *PVOID;
			 * typedef PVOID HANDLE;
			 * typedef HANDLE HWND;
			 * So it's safe to treat it as a unsigned int, even
			 * though silly MSVC will complain. If MSVC is unhappy,
			 * why not tell us how to handle it correctly?
			 * Seems Windows print HWND as a decimal number,
			 * so %p with scanf() is not suitable here.
			 */
			app->preview_window = strtoul(argopt, NULL, 0);
			break;
#	endif
		/**
		 * Windows taken `/s` for screensaver, and if you run `.scr`
		 * file from explorer, it will add `/S`! What a silly behavior,
		 * so we cannot use those chars.
		 */
		case '3':
			app->show_second = true;
			break;
		case 'w':
			app->full = false;
			break;
		case 't':
			if (argopt == NULL) {
				LOG_ERROR("Missing value for option `%c%c`\n",
					  OPT_START, opt);
				exit_after_argument = true;
				break;
			}
			if (atoi(argopt) == 12)
				app->ampm = true;
			else
				app->ampm = false;
			break;
		case 'f':
			if (argopt == NULL) {
				LOG_ERROR("Missing value for option `%c%c`\n",
					  OPT_START, opt);
				exit_after_argument = true;
				break;
			}
			strncpy(app->font_path, argopt, MAX_BUFFER_LENGTH);
			app->font_path[MAX_BUFFER_LENGTH - 1] = '\0';
			if (strlen(app->font_path) == MAX_BUFFER_LENGTH - 1)
				LOG_ERROR("`font_path` too long, "
					  "may fail to load.\n");
			break;
		case 0:
			LOG_ERROR("%s: Invalid value `%s`.\n", argv[0], argopt);
			break;
		default:
			LOG_ERROR("%s: Invalid option `%c%c`.\n", argv[0],
				  OPT_START, opt);
			break;
		}
	}
	if (exit_after_argument)
		goto exit;
#endif

	flipclock_create_clocks(app);

	flipclock_run_mainloop(app);

	flipclock_destroy_clocks(app);

exit:
	flipclock_destroy(app);
	TTF_Quit();
	SDL_Quit();
	return 0;
}
