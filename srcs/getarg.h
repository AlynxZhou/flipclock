/**
 * Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.one/)
 */
#ifndef __GETARG_H__
#define __GETARG_H__

#ifdef _WIN32
#	define OPT_START '/'
#else
#	define OPT_START '-'
#endif

#define FORCE_STOP_OPTS "--"

// Global optarg.
extern char *argopt;

/**
 * `getarg()`: a portable option parser similiar to `getopt()`.
 *
 * Just use it like `getopt()`, but with following differences:
 * - Global variable `optarg` is replaced with `argopt`.
 * - `getopt()` will return `'?'` if it gets an option character not in
 *   `optstring`, `getarg()` will return the character.
 * - `getopt()` will return `'?'` or `':'` if it gets an option character with a
 *    missing argument string, `getarg()` will return the character, you should
 *    check whether `argopt` is `NULL` by yourself.
 * - `getopt()` will return `-1` if it gets `"--"` and give you a global
 *    variable `optind` to handle remaining argument strings, `getarg()` does
 *    not return `-1` on `"--"`, but it will stop option-parsing, you can
 *    continue calling it in a loop to get next argument strings in `argopt`,
 *    it will return `0` for them.
 */
int getarg(int argc, char *argv[], const char optstring[]);

#endif
