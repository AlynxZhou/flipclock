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

	/* Global optarg. */
	extern char *optarg;

	/*
	 * get_arg() receives an arguments counter,
	 * a pointer array of arguments,
	 * and parse it with given option string.
	 * It only receives single-char option, and will return it.
	 * In order to parse a value following the option,
	 * use "x:" in the option string where 'x' is the option,
	 * and it will store a pointer to the value in global pointer optarg.
	 * You should use a while-switch-case to deal with options.
	 */
	int get_arg(int argc, char *argv[], char opt_string[]);

#endif
