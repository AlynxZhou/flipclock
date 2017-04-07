/*
 * Filename: getarg.c
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 * This is a simple and cross-platform version getArg() inspired by POSIX
 * getopt() which could not work with normal Microsoft's compiler.
 * If you find any bugs with this, please tell me because I'm a student who
 * major in computer science and want to be more experienced!
 */
#include "getarg.h"

const char *optarg = NULL;

int getArg(int argc, const char *argv[], const char optstring[])
{
	static int i = 0, j = 1;
	// Always init j with 1, because 0 is '-'.
	while (i < argc) {
		if (argv[i][0] != '-' || argv[i][j] == '\0') {
			// Arguments begin with '-' and end with '\0'.
			i++;
			j = 1;
			continue;
		} else if (strchr(optstring, argv[i][j]) == NULL) {
			// Not a valid argument.
			fprintf(stderr, "%s: Invalid argument \"-%c\".\n", argv[0], argv[i][j]);
			exit(EXIT_FAILURE);
		} else if (strchr(optstring, argv[i][j]) != NULL && *(strchr(optstring, argv[i][j]) + 1) != ':') {
			// Arguments not finished.
			int tempI = i;
			int tempJ = j++;
			return argv[tempI][tempJ];
		} else if (*(strchr(optstring, argv[i][j]) + 1) == ':') {
			// An argument followed by a value.
			if (i + 1 < argc  && argv[i][j + 1] == '\0') {
				// The argument must be followed by a value, or it will be skipped.
				int tempI = i++;
				int tempJ = j;
				optarg = argv[i];
				j = 1;
				return argv[tempI][tempJ];
			} else {
				// Just skip an argument with ':' but no value.
				j++;
			}
		}
	}
	return -1;
}

/*
int main(int argc, char const *argv[]) {
	const char optstring[] = "ht:f";
	for (int opt = getArg(argc, argv, optstring); opt != -1; opt = getArg(argc, argv, optstring)) {
		switch(opt) {
			case 'f':
				printf("Argument \"-%c\".\n", opt);
				break;
			case 't':
				printf("Argument \"-%c\" with \"%s\".\n", opt, optarg);
				break;
			case 'h':
				printf("Argument \"-%c\".\n", opt);
				break;
			default:
        			break;
        	}
    	}
	return 0;
}
*/
