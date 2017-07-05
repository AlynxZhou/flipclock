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
	const char VERSION[] = "2.3.3";
	struct app_all flipclock;
	/* Fill default content. */
	fill_default(&flipclock);
	flipclock.properties.title = TITLE;
	flipclock.properties.version = VERSION;
	flipclock.properties.fallback_font = FALLBACK_FONT;
	flipclock.properties.program_name = argv[0];
	/* Dealing with arguments. */
	int opt;
	while ((opt = getarg(argc, argv, OPT_STRING)) != -1) {
		switch (opt) {
		case 'w':
			flipclock.properties.full = false;
			break;
		case 't':
			if (strcmp(argopt, "12") == 0)
				flipclock.properties.ampm = true;
			break;
		case 'f':
			flipclock.properties.font_path = argopt;
			break;
		case 's':
			sscanf(argopt, "%lf", \
			       &flipclock.properties.scale);
			break;
		case 'h':
			print_help(&flipclock);
			exit(EXIT_SUCCESS);
			break;
		case 0:
			fprintf(stderr, "%s: Invalid value `%s`.\n", \
				argv[0], argopt);
			exit(EXIT_FAILURE);
			break;
		default:
			fprintf(stderr, "%s: Invalid option `%c%c`.\n", \
				argv[0], OPT_START, opt);
			exit(EXIT_FAILURE);
			break;
        	}
    	}
	/* Try to init app. */
	if (!init_app(&flipclock)) {
		quit_app(&flipclock);
		exit(EXIT_FAILURE);
	}
	/* Route and handle events. */
	route_event(&flipclock, TIMEOUT);
	/* Clean and quit. */
	quit_app(&flipclock);
	return 0;
}
