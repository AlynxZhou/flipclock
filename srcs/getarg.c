/**
 * Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.one/)
 */
#include <string.h>

#include "getarg.h"

char *argopt = NULL;

// TODO: Make it more similiar to getopt().
int getarg(int argc, char *argv[], const char optstring[])
{
	static int force_stopped = 0;
	// Always init i with 1, because 0 is the program name.
	static int i = 1;
	// Always init j with 1, because 0 is OPT_START.
	static int j = 1;
	// Temp i and j for an option followed by a value.
	int temp_i = 0;
	int temp_j = 0;

	while (i < argc) {
		argopt = NULL;

		// `"--"` forces an end of option parsing, all remaining
		// arguments are treated as values, so program can handle values
		// starting with `'-'`.
		if (force_stopped) {
			argopt = argv[i];
			++i;
			return 0;
		} else if (!strcmp(argv[i], FORCE_STOP_OPTS)) {
			force_stopped = 1;
			// Skip `"--"`.
			++i;
			continue;
		}

		// Not force_stopped, do option parsing.

		// A string is finished, jump to next.
		if (argv[i][j] == '\0') {
			++i;
			j = 1;
			continue;
		}

		// We jump to a string for the first time and it's just a value.
		if (j == 1 && argv[i][0] != OPT_START) {
			argopt = argv[i];
			++i;
			return 0;
		}

		char *chrptr = strchr(optstring, argv[i][j]);
		if (chrptr == NULL || *(chrptr + 1) != ':') {
			// Not a valid option, but just return it. Or an option
			// without value, which means options not finished.
			temp_j = j;
			++j;
			return argv[i][temp_j];
		} else {
			// With value.
			if (argv[i][j + 1] != '\0') {
				// gcc style `"-Wall"`.
				temp_i = i;
				temp_j = j;
				argopt = &argv[i][j + 1];
				++i;
				j = 1;
				return argv[temp_i][temp_j];
			} else if (i + 1 < argc) {
				// `"-W all"`.
				temp_i = i;
				temp_j = j;
				argopt = argv[i + 1];
				// Skip value, so add 2 to i.
				i += 2;
				j = 1;
				return argv[temp_i][temp_j];
			} else {
				// Last argument is `"-W"` without any value.
				// It's safe to just increase j, we will go into
				// `'\0'` to increase i and then break the loop.
				temp_j = j;
				++j;
				return argv[i][temp_j];
			}
		}
	}

	return -1;
}
