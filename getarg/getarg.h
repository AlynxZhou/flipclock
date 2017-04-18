/*
 * Filename: getarg.h
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 */
#ifndef _GETARG_H
#	define _GETARG_H
#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>

	// Global optarg.
	extern char *optarg;

	int get_arg(int argc, char *argv[], char opt_string[]);

#endif
