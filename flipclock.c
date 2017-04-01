/*
 * Filename: flipclock.c
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "SDL.h"
#include "SDL_ttf.h"

#define TITLE "FlipClock"
#define WIDTH 1024
#define HEIGHT 768

bool ampm = false;
bool full = false;
