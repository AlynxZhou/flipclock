/*
 * Filename: getarg.h
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 */
#ifndef __GETARG_H__
#	define __GETARG_H__
#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>

#	ifndef WIN32
#		define ARG_START '-'
#	else
#		define ARG_START '/'
#	endif

/* Global optarg. */
extern const char *optarg;

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
int getarg(const int argc, \
	   const char *const argv[], \
	   const char opt_string[]);

#endif
