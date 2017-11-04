/*
 * Filename: getarg.c
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 */
#include <string.h>

#include "getarg.h"

const char *argopt = NULL;

int getarg(const int argc, \
	   const char *const argv[], \
	   const char opt_string[])
{
	static int i = 1;
	/* Always init i with 1, because 0 is the program name. */
	static int j = 1;
	/* Always init j with 1, because 0 is OPT_START. */
	int temp_i = 0;
	int temp_j = 0;
	/* Temp i and j for an option followed by a value. */
	while (i < argc) {
		argopt = NULL;
		if (argv[i][0] != OPT_START) {
			/*
			 * If there is a single value not leading by
			 * an option, then argopt will be pointed to
			 * it and return 0.
			 */
			argopt = argv[i++];
			return 0;
		} else if (argv[i][j] == '\0') {
			/*
			 * All options must begin with OPT_START
			 * and end with '\0'.
			 */
			i++;
			j = 1;
			continue;
		} else if (strchr(opt_string, argv[i][j]) == NULL) {
			/* Not a valid option. But just return it. */
			return argv[i][j++];
		} else if (*(strchr(opt_string, argv[i][j]) + 1) != ':') {
			/* Options not finished. */
			return argv[i][j++];
		} else if (*(strchr(opt_string, argv[i][j]) + 1) == ':') {
			/* An option followed by a value. */
			if (i + 1 < argc && argv[i][j + 1] == '\0') {
				/*
				 * The option must be followed
				 * by a value, or it will be skipped.
				 */
				temp_i = i;
 				temp_j = j;
				argopt = argv[++i];
				i++;
				/* Increase i to skip value in next loop. */
 				j = 1;
 				return argv[temp_i][temp_j];
			} else {
				/*
				 * Just skip an option with
				 * ':' but no value.
				 */
				j++;
			}
		}
	}
	return -1;
}
