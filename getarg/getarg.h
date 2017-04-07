/*
 * Filename: getarg.c
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 * This is a simple and cross-platform version getArg() inspired by POSIX
 * getopt() which could not work with normal Microsoft's compiler.
 * If you find any bugs with this, please tell me because I'm a student who
 * major in computer science and want to be more experienced!
 */
#ifndef _GETARG_H
#	define _GETARG_H
#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>

	extern const char *optarg;
	int getArg(int argc, const char *argv[], const char optstring[]);

#endif
