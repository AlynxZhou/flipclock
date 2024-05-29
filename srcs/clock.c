#include "srcs/card.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "flipclock.h"
#include "clock.h"
#include "card.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

static void _flipclock_clock_update_layout(struct flipclock_clock *clock)
{
	RETURN_IF_FAIL(clock != NULL);

	const struct flipclock *app = clock->app;
	SDL_Rect hour_rect;
	SDL_Rect minute_rect;
	SDL_Rect second_rect;
	int cards_length = app->show_second ? 3 : 2;
	int spaces_length = cards_length + 1;
	// space/card = 1/8.
	/**
	 * In best condition, we have 1 + 8 + 1 + 8 + 1. However, the other
	 * length of window might be smaller, and the card is less than 8. We
	 * will enlarge the spaces of begining and end, so only care about
	 * spaces between cards when calculating position.
	 */
	if (clock->w >= clock->h) {
		int space_size = clock->w / (cards_length * 8 + spaces_length);
		int min_height = clock->h * 0.8;
		int min_width =
			clock->w * 8 / (cards_length * 8 + spaces_length);
		int card_size = min_height < min_width ? min_height : min_width;
		card_size *= app->card_scale;

		hour_rect.x = (clock->w - card_size * cards_length -
			       space_size * (spaces_length - 2)) /
			      2;
		hour_rect.y = (clock->h - card_size) / 2;
		hour_rect.w = card_size;
		hour_rect.h = card_size;
		flipclock_card_set_rect(clock->hour, hour_rect);

		minute_rect.x = hour_rect.x + hour_rect.w + space_size;
		minute_rect.y = hour_rect.y;
		minute_rect.w = card_size;
		minute_rect.h = card_size;
		flipclock_card_set_rect(clock->minute, minute_rect);

		if (app->show_second) {
			second_rect.x = hour_rect.x + hour_rect.w + space_size +
					minute_rect.w + space_size;
			second_rect.y = hour_rect.y;
			second_rect.w = card_size;
			second_rect.h = card_size;
			flipclock_card_set_rect(clock->second, second_rect);
		}
	} else {
		int space_size = clock->h / (cards_length * 8 + spaces_length);
		int min_width = clock->w * 0.8;
		int min_height =
			clock->h * 8 / (cards_length * 8 + spaces_length);
		int card_size = min_height < min_width ? min_height : min_width;
		card_size *= app->card_scale;

		hour_rect.x = (clock->w - card_size) / 2;
		hour_rect.y = (clock->h - card_size * cards_length -
			       space_size * (spaces_length - 2)) /
			      2;
		hour_rect.w = card_size;
		hour_rect.h = card_size;
		flipclock_card_set_rect(clock->hour, hour_rect);

		minute_rect.y = hour_rect.y + hour_rect.h + space_size;
		minute_rect.x = hour_rect.x;
		minute_rect.w = card_size;
		minute_rect.h = card_size;
		flipclock_card_set_rect(clock->minute, minute_rect);

		if (app->show_second) {
			second_rect.y = hour_rect.y + hour_rect.h + space_size +
					minute_rect.h + space_size;
			second_rect.x = hour_rect.x;
			second_rect.w = card_size;
			second_rect.h = card_size;
			flipclock_card_set_rect(clock->second, second_rect);
		}
	}
}

static void _flipclock_clock_create_cards(struct flipclock_clock *clock)
{
	RETURN_IF_FAIL(clock != NULL);

	struct flipclock *app = clock->app;

	clock->hour = flipclock_card_create(app, clock->renderer);
	clock->minute = flipclock_card_create(app, clock->renderer);
	clock->second = NULL;
	if (app->show_second)
		clock->second = flipclock_card_create(app, clock->renderer);
	_flipclock_clock_update_layout(clock);
}

struct flipclock_clock *flipclock_clock_create(struct flipclock *app, int i)
{
	RETURN_VAL_IF_FAIL(app != NULL, NULL);

	/**
	 * We need `SDL_WINDOW_RESIZABLE` for auto-rotate while fullscreen on
	 * Android.
	 *
	 * Being HiDPI aware aware on macOS requires extra files, we don't have
	 * it so disable this currently.
	 */
	unsigned int flags = SDL_WINDOW_SHOWN |
#ifndef __APPLE__
		SDL_WINDOW_ALLOW_HIGHDPI ｜
#endif
		SDL_WINDOW_RESIZABLE;
	if (app->full) {
		flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
#ifndef __APPLE__
			SDL_WINDOW_ALLOW_HIGHDPI |
#endif
			SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	struct flipclock_clock *clock = malloc(sizeof(*clock));
	if (clock == NULL) {
		LOG_ERROR("Failed to create clock!");
		exit(EXIT_FAILURE);
	}
	clock->app = app;
	clock->waiting = false;
	clock->i = i;
	SDL_Rect display_bounds;
	SDL_GetDisplayBounds(i, &display_bounds);
	// Give each window a unique title.
	char window_title[MAX_BUFFER_LENGTH];
	snprintf(window_title, MAX_BUFFER_LENGTH, PROGRAM_TITLE " %d", i);
	if (app->full)
		clock->window = SDL_CreateWindow(window_title, display_bounds.x,
						 display_bounds.y,
						 display_bounds.w,
						 display_bounds.h, flags);
	else
		clock->window = SDL_CreateWindow(
			window_title,
			display_bounds.x +
				(display_bounds.w - WINDOW_WIDTH) / 2,
			display_bounds.y +
				(display_bounds.h - WINDOW_HEIGHT) / 2,
			WINDOW_WIDTH, WINDOW_HEIGHT, flags);
	if (clock->window == NULL) {
		LOG_ERROR("%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	// Get actual window size after create it.
	SDL_GetWindowSize(clock->window, &clock->w, &clock->h);
	clock->renderer = SDL_CreateRenderer(
		clock->window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE |
			SDL_RENDERER_PRESENTVSYNC);
	if (clock->renderer == NULL) {
		LOG_ERROR("%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetRenderDrawBlendMode(clock->renderer, SDL_BLENDMODE_BLEND);
	_flipclock_clock_create_cards(clock);
	return clock;
}

#if defined(_WIN32)
/**
 * Create clock from given HWND, which should be a subwindow of screensaver
 * preview.
 */
struct flipclock_clock *flipclock_clock_create_preview(struct flipclock *app)
{
	RETURN_VAL_IF_FAIL(app != NULL, NULL);

	struct flipclock_clock *clock = malloc(sizeof(*clock));
	if (clock == NULL) {
		LOG_ERROR("Failed to create clock!");
		exit(EXIT_FAILURE);
	}
	clock->app = app;
	clock->waiting = false;
	clock->i = 0;
	clock->window = SDL_CreateWindowFrom(app->preview_window);
	if (clock->window == NULL) {
		LOG_ERROR("%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	// Get actual window size after create it.
	SDL_GetWindowSize(clock->window, &clock->w, &clock->h);
	clock->renderer = SDL_CreateRenderer(
		clock->window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE |
			SDL_RENDERER_PRESENTVSYNC);
	if (clock->renderer == NULL) {
		LOG_ERROR("%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetRenderDrawBlendMode(clock->renderer, SDL_BLENDMODE_BLEND);
	_flipclock_clock_create_cards(clock);
	return clock;
}
#endif

void flipclock_clock_set_show_second(struct flipclock_clock *clock,
				     bool show_second)
{
	RETURN_IF_FAIL(clock != NULL);

	if (show_second) {
		if (clock->second == NULL)
			clock->second = flipclock_card_create(clock->app,
							      clock->renderer);
	} else {
		if (clock->second != NULL) {
			flipclock_card_destory(clock->second);
			clock->second = NULL;
		}
	}
	// Toggling seconds always changes size.
	_flipclock_clock_update_layout(clock);
}

void flipclock_clock_set_fullscreen(struct flipclock_clock *clock, bool full)
{
	RETURN_IF_FAIL(clock != NULL);

	// Let's find which display the clock is inside.
	SDL_Rect display_bounds;
	int clock_x;
	int clock_y;
	SDL_GetWindowPosition(clock->window, &clock_x, &clock_y);
	int clock_center_x = clock_x + clock->w / 2;
	int clock_center_y = clock_y + clock->h / 2;
	int displays_length = SDL_GetNumVideoDisplays();
	// If a clock is out of all displays it will be re-placed into the last.
	for (int i = 0; i < displays_length; ++i) {
		SDL_GetDisplayBounds(i, &display_bounds);
		if (clock_center_x >= display_bounds.x &&
		    clock_center_x < display_bounds.x + display_bounds.w &&
		    clock_center_y >= display_bounds.y &&
		    clock_center_y < display_bounds.y + display_bounds.h) {
			LOG_DEBUG("Clock `%d` is inside display `%d`.\n",
				  clock->i, i);
			break;
		}
	}
	if (full) {
		// Move clocks to their placed displays.
		SDL_SetWindowPosition(clock->window, display_bounds.x,
				      display_bounds.y);
		SDL_SetWindowFullscreen(clock->window,
					SDL_WINDOW_FULLSCREEN_DESKTOP);
		SDL_GetWindowSize(clock->window, &clock->w, &clock->h);
		LOG_DEBUG("Set clock `%d` to fullscreen with size `%dx%d`.\n",
			  clock->i, clock->w, clock->h);
	} else {
		SDL_SetWindowFullscreen(clock->window, 0);
		/**
		 * We need to restore window first, because if started in
		 * fullscreen mode, it will be maximized when turning off
		 * fullscreen mode and we cannot set window size. Looks like
		 * a strange bug.
		 */
		SDL_RestoreWindow(clock->window);
		SDL_SetWindowSize(clock->window, WINDOW_WIDTH, WINDOW_HEIGHT);
		SDL_SetWindowPosition(
			clock->window,
			display_bounds.x +
				(display_bounds.w - WINDOW_WIDTH) / 2,
			display_bounds.y +
				(display_bounds.h - WINDOW_HEIGHT) / 2);
		SDL_GetWindowSize(clock->window, &clock->w, &clock->h);
		LOG_DEBUG("Set clock `%d` to windowed.\n", clock->i);
	}
	// Toggling fullscreen always changes size.
	_flipclock_clock_update_layout(clock);
}

void flipclock_clock_set_hour(struct flipclock_clock *clock, const char hour[],
			      bool flip)
{
	// Text can be NULL to clear card.
	RETURN_IF_FAIL(clock != NULL);

	flipclock_card_set_text(clock->hour, hour);
	if (flip)
		flipclock_card_flip(clock->hour);
}

void flipclock_clock_set_minute(struct flipclock_clock *clock,
				const char minute[], bool flip)
{
	// Text can be NULL to clear card.
	RETURN_IF_FAIL(clock != NULL);

	flipclock_card_set_text(clock->minute, minute);
	if (flip)
		flipclock_card_flip(clock->minute);
}

void flipclock_clock_set_second(struct flipclock_clock *clock,
				const char second[], bool flip)
{
	// Text can be NULL to clear card.
	RETURN_IF_FAIL(clock != NULL);

	if (!clock->app->show_second)
		return;

	flipclock_card_set_text(clock->second, second);
	if (flip)
		flipclock_card_flip(clock->second);
}

void flipclock_clock_set_ampm(struct flipclock_clock *clock, const char ampm[])
{
	// Text can be NULL to clear card.
	RETURN_IF_FAIL(clock != NULL);

	flipclock_card_set_sub_text(clock->hour, ampm);
	// Set ampm should never flip a card.
}

void flipclock_clock_handle_window_event(struct flipclock_clock *clock,
					 SDL_Event event)
{
	RETURN_IF_FAIL(clock != NULL);

	const struct flipclock *app = clock->app;
	int clock_i = clock->i;
	switch (event.window.event) {
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		/**
		 * Only re-render when size changed.
		 * Windows may send event when size
		 * not changed, and cause strange bugs.
		 */
		if (event.window.data1 != clock->w ||
		    event.window.data2 != clock->h) {
			clock->w = event.window.data1;
			clock->h = event.window.data2;
			LOG_DEBUG("New window size for "
				  "clock `%d` is `%dx%d`.\n",
				  clock->i, clock->w, clock->h);
			_flipclock_clock_update_layout(clock);
		}
		break;
	case SDL_WINDOWEVENT_MINIMIZED:
		clock->waiting = true;
		break;
	// `RESTORED` is emitted after `MINIMIZED`.
	case SDL_WINDOWEVENT_RESTORED:
		clock->waiting = false;
		/**
		 * Sometimes when a window is restored, its texture get lost.
		 * Typically happens when we have two fullscreen clocks in
		 * one display, and the lower one is switched to top, and we
		 * have to redraw its texture.
		 */
		// XXX: It seems OK without redrawing.
		// flipclock_clock_draw(clock);
		break;
	case SDL_WINDOWEVENT_CLOSE:
		flipclock_clock_destroy(clock);
		app->clocks[clock_i] = NULL;
		/**
		 * See <https://wiki.libsdl.org/SDL_EventType#SDL_QUIT>.
		 * It seems that SDL will send SDL_QUIT automatically
		 * when all windows are closed, so we don't need to exit
		 * manually here.
		 */
		LOG_DEBUG("Clock `%d` closed!\n", clock_i);
		break;
	default:
		break;
	}
}

void flipclock_clock_animate(struct flipclock_clock *clock)
{
	RETURN_IF_FAIL(clock != NULL);

	const struct flipclock *app = clock->app;
	SDL_SetRenderDrawColor(clock->renderer, app->background_color.r,
			       app->background_color.g, app->background_color.b,
			       app->background_color.a);
	SDL_RenderClear(clock->renderer);

	flipclock_card_animate(clock->hour);
	flipclock_card_animate(clock->minute);
	if (app->show_second)
		flipclock_card_animate(clock->second);

	SDL_RenderPresent(clock->renderer);
}

void flipclock_clock_destroy(struct flipclock_clock *clock)
{
	RETURN_IF_FAIL(clock != NULL);

	flipclock_card_destory(clock->hour);
	flipclock_card_destory(clock->minute);
	if (clock->second != NULL)
		flipclock_card_destory(clock->second);
	SDL_DestroyRenderer(clock->renderer);
	SDL_DestroyWindow(clock->window);
	free(clock);
}
