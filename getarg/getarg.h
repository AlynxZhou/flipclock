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

	extern const char *optarg;
	int getArg(int argc, const char *argv[], const char optstring[]);

#endif
