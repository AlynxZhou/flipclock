/*
 * Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.moe/)
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "getarg.h"
#include "flipclock.h"

#include "config.h"

#define FPS 60
#define MAX_PROGRESS 300
#define HALF_PROGRESS (MAX_PROGRESS / 2)
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct flipclock *flipclock_create(void)
{
	struct flipclock *app = malloc(sizeof(*app));
	if (app == NULL) {
		fprintf(stderr, "Failed to create app!\n");
		exit(EXIT_FAILURE);
	}
	app->clocks = NULL;
	/* Should create 1 clock in windowed mode. */
	app->clocks_length = 1;
	app->colors.font.r = 0xd0;
	app->colors.font.g = 0xd0;
	app->colors.font.b = 0xd0;
	app->colors.font.a = 0xff;
	app->colors.rect.r = 0x20;
	app->colors.rect.g = 0x20;
	app->colors.rect.b = 0x20;
	app->colors.rect.a = 0xff;
	app->colors.black.r = 0x00;
	app->colors.black.g = 0x00;
	app->colors.black.b = 0x00;
	app->colors.black.a = 0xff;
	app->colors.transparent.r = 0x00;
	app->colors.transparent.g = 0x00;
	app->colors.transparent.b = 0x00;
	app->colors.transparent.a = 0x00;
	app->properties.ampm = false;
	app->properties.full = true;
	app->properties.font_path = NULL;
#ifdef _WIN32
	app->properties.preview = false;
	app->properties.screensaver = false;
#endif
	time_t raw_time = time(NULL);
	app->times.past = *localtime(&raw_time);
	app->times.now = *localtime(&raw_time);
	return app;
}

void flipclock_create_clocks(struct flipclock *app)
{
#ifdef _WIN32
	if (!app->properties.screensaver)
		SDL_DisableScreenSaver();
	if (app->properties.preview) {
		/* Don't set fullscreen if in preview. */
		app->properties.full = false;
		app->clocks = malloc(sizeof(*app->clocks) * app->clocks_length);
		/* Create window from native window when in preview. */
		app->clocks[0].window =
			SDL_CreateWindowFrom(app->properties.preview_window);
	} else {
		unsigned int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
				     SDL_WINDOW_ALLOW_HIGHDPI;
		if (app->properties.full) {
			app->clocks_length = SDL_GetNumVideoDisplays();
			flags = SDL_WINDOW_SHOWN |
				SDL_WINDOW_FULLSCREEN_DESKTOP |
				SDL_WINDOW_ALLOW_HIGHDPI;
			SDL_ShowCursor(SDL_DISABLE);
		}
		app->clocks = malloc(sizeof(*app->clocks) * app->clocks_length);
		for (int i = 0; i < app->clocks_length; ++i) {
			app->clocks[i].fonts.time = NULL;
			app->clocks[i].fonts.mode = NULL;
			app->clocks[i].textures.current = NULL;
			app->clocks[i].textures.previous = NULL;
			app->clocks[i].wait = false;
			SDL_Rect display_bounds;
			SDL_GetDisplayBounds(i, &display_bounds);
			app->clocks[i].window = SDL_CreateWindow(PROGRAM_TITLE,
				display_bounds.x,
				display_bounds.y,
				display_bounds.w,
				display_bounds.h, flags);
			if (app->clocks[i].window == NULL) {
				fprintf(stderr, "%s\n", SDL_GetError());
				exit(EXIT_FAILURE);
			}
			/* Init window size after create it. */
			SDL_GetWindowSize(app->clocks[i].window, &app->clocks[i].width,
					  &app->clocks[i].height);
			app->clocks[i].renderer = SDL_CreateRenderer(app->clocks[i].window, -1,
								   SDL_RENDERER_ACCELERATED |
									   SDL_RENDERER_TARGETTEXTURE |
									   SDL_RENDERER_PRESENTVSYNC);
			if (app->clocks[i].renderer == NULL) {
				fprintf(stderr, "%s\n", SDL_GetError());
				exit(EXIT_FAILURE);
			}
			SDL_SetRenderDrawBlendMode(app->clocks[i].renderer, SDL_BLENDMODE_BLEND);
		}
	}
#else
	SDL_DisableScreenSaver();
	unsigned int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
			     SDL_WINDOW_ALLOW_HIGHDPI;
	if (app->properties.full) {
		app->clocks_length = SDL_GetNumVideoDisplays();
		flags = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP |
			SDL_WINDOW_ALLOW_HIGHDPI;
		SDL_ShowCursor(SDL_DISABLE);
	}
	app->clocks = malloc(sizeof(*(app->clocks)) * app->clocks_length);
	for (int i = 0; i < app->clocks_length; ++i) {
		app->clocks[i].fonts.time = NULL;
		app->clocks[i].fonts.mode = NULL;
		app->clocks[i].textures.current = NULL;
		app->clocks[i].textures.previous = NULL;
		app->clocks[i].wait = false;
		SDL_Rect display_bounds;
		SDL_GetDisplayBounds(i, &display_bounds);
		app->clocks[i].window = SDL_CreateWindow(PROGRAM_TITLE,
			display_bounds.x,
			display_bounds.y,
			display_bounds.w,
			display_bounds.h, flags);
		if (app->clocks[i].window == NULL) {
			fprintf(stderr, "%s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		/* Init window size after create it. */
		SDL_GetWindowSize(app->clocks[i].window, &app->clocks[i].width,
				  &app->clocks[i].height);
		app->clocks[i].renderer = SDL_CreateRenderer(app->clocks[i].window, -1,
							   SDL_RENDERER_ACCELERATED |
								   SDL_RENDERER_TARGETTEXTURE |
								   SDL_RENDERER_PRESENTVSYNC);
		if (app->clocks[i].renderer == NULL) {
			fprintf(stderr, "%s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		SDL_SetRenderDrawBlendMode(app->clocks[i].renderer, SDL_BLENDMODE_BLEND);
	}
#endif
}

void flipclock_set_fullscreen(struct flipclock *app, int clock_index, bool full)
{
	app->properties.full = full;
	if (full) {
		SDL_SetWindowFullscreen(app->clocks[clock_index].window,
			SDL_WINDOW_FULLSCREEN_DESKTOP);
		SDL_GetWindowSize(app->clocks[clock_index].window, &app->clocks[clock_index].width,
			&app->clocks[clock_index].height);
		SDL_ShowCursor(SDL_DISABLE);
	} else {
		SDL_SetWindowFullscreen(app->clocks[clock_index].window, 0);
		SDL_GetWindowSize(app->clocks[clock_index].window, &app->clocks[clock_index].width,
			&app->clocks[clock_index].height);
		/* Move mouse back into center. */
		SDL_WarpMouseInWindow(app->clocks[clock_index].window, app->clocks[clock_index].width / 2,
			app->clocks[clock_index].height / 2);
		SDL_ShowCursor(SDL_ENABLE);
	}
}

void flipclock_refresh(struct flipclock *app, int clock_index)
{
	if (app->clocks[clock_index].width < app->clocks[clock_index].height) {
		/* Some user do love portrait. */
		app->clocks[clock_index].rect_size =
			app->clocks[clock_index].height * 0.4 >
					app->clocks[clock_index].width * 0.8 ?
				app->clocks[clock_index].width * 0.8 :
				app->clocks[clock_index].height * 0.4;
		int space = app->clocks[clock_index].height * 0.06;
		app->clocks[clock_index].radius = app->clocks[clock_index].rect_size / 10;

		app->clocks[clock_index].rects.hour.y = (app->clocks[clock_index].height -
				     2 * app->clocks[clock_index].rect_size - space) /
				    2;
		app->clocks[clock_index].rects.hour.x =
			(app->clocks[clock_index].width - app->clocks[clock_index].rect_size) / 2;
		app->clocks[clock_index].rects.hour.w = app->clocks[clock_index].rect_size;
		app->clocks[clock_index].rects.hour.h = app->clocks[clock_index].rect_size;

		app->clocks[clock_index].rects.minute.y =
			app->clocks[clock_index].rects.hour.y + app->clocks[clock_index].rects.hour.h + space;
		app->clocks[clock_index].rects.minute.x = app->clocks[clock_index].rects.hour.x;
		app->clocks[clock_index].rects.minute.w = app->clocks[clock_index].rect_size;
		app->clocks[clock_index].rects.minute.h = app->clocks[clock_index].rect_size;
	} else {
		/* But others love landscape. */
		app->clocks[clock_index].rect_size =
			app->clocks[clock_index].width * 0.4 >
					app->clocks[clock_index].height * 0.8 ?
				app->clocks[clock_index].height * 0.8 :
				app->clocks[clock_index].width * 0.4;
		int space = app->clocks[clock_index].width * 0.06;
		app->clocks[clock_index].radius = app->clocks[clock_index].rect_size / 10;

		app->clocks[clock_index].rects.hour.x = (app->clocks[clock_index].width -
				     2 * app->clocks[clock_index].rect_size - space) /
				    2;
		app->clocks[clock_index].rects.hour.y =
			(app->clocks[clock_index].height - app->clocks[clock_index].rect_size) /
			2;
		app->clocks[clock_index].rects.hour.w = app->clocks[clock_index].rect_size;
		app->clocks[clock_index].rects.hour.h = app->clocks[clock_index].rect_size;

		app->clocks[clock_index].rects.minute.x =
			app->clocks[clock_index].rects.hour.x + app->clocks[clock_index].rects.hour.w + space;
		app->clocks[clock_index].rects.minute.y = app->clocks[clock_index].rects.hour.y;
		app->clocks[clock_index].rects.minute.w = app->clocks[clock_index].rect_size;
		app->clocks[clock_index].rects.minute.h = app->clocks[clock_index].rect_size;
	}

	/* How do I get those numbers? Test. */
	app->clocks[clock_index].rects.mode.w = app->clocks[clock_index].rect_size / 5;
	app->clocks[clock_index].rects.mode.h = app->clocks[clock_index].rect_size / 10;
	app->clocks[clock_index].rects.mode.x = app->clocks[clock_index].rects.hour.x + app->clocks[clock_index].rect_size / 50;
	app->clocks[clock_index].rects.mode.y = app->clocks[clock_index].rects.hour.y + app->clocks[clock_index].rect_size -
			    app->clocks[clock_index].rects.mode.h - app->clocks[clock_index].rect_size / 35;
}

void flipclock_create_textures(struct flipclock *app, int clock_index)
{
	/* Two transparent backend texture swap for tribuffer. */
	app->clocks[clock_index].textures.current = SDL_CreateTexture(app->clocks[clock_index].renderer, 0,
						  SDL_TEXTUREACCESS_TARGET,
						  app->clocks[clock_index].width,
						  app->clocks[clock_index].height);
	if (app->clocks[clock_index].textures.current == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetTextureBlendMode(app->clocks[clock_index].textures.current, SDL_BLENDMODE_BLEND);
	app->clocks[clock_index].textures.previous = SDL_CreateTexture(app->clocks[clock_index].renderer, 0,
						   SDL_TEXTUREACCESS_TARGET,
						   app->clocks[clock_index].width,
						   app->clocks[clock_index].height);
	if (app->clocks[clock_index].textures.previous == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetTextureBlendMode(app->clocks[clock_index].textures.previous, SDL_BLENDMODE_BLEND);
}

void flipclock_destroy_textures(struct flipclock *app, int clock_index)
{
	if (app->clocks[clock_index].textures.current != NULL)
		SDL_DestroyTexture(app->clocks[clock_index].textures.current);
	if (app->clocks[clock_index].textures.previous != NULL)
		SDL_DestroyTexture(app->clocks[clock_index].textures.previous);
}

void flipclock_open_fonts(struct flipclock *app, int clock_index)
{
	if (app->properties.font_path != NULL) {
		app->clocks[clock_index].fonts.time = TTF_OpenFont(app->properties.font_path,
					       app->clocks[clock_index].rect_size);
		app->clocks[clock_index].fonts.mode = TTF_OpenFont(app->properties.font_path,
					       app->clocks[clock_index].rects.mode.h);
	} else {
#ifdef _WIN32
		char *system_root = getenv("SystemRoot");
		int path_size = strlen(system_root) +
				strlen("\\Fonts\\flipclock.ttf") + 1;
		char *font_path = malloc(path_size * sizeof(*font_path));
		if (font_path == NULL) {
			fprintf(stderr, "Load font path failed!\n");
			exit(EXIT_FAILURE);
		}
		strncpy(font_path, system_root, strlen(system_root) + 1);
		strncat(font_path, "\\Fonts\\flipclock.ttf",
			strlen("\\Fonts\\flipclock.ttf") + 1);
#else
		char font_path[] = CMAKE_INSTALL_PREFIX
			"/share/fonts/flipclock.ttf";
#endif
		app->clocks[clock_index].fonts.time =
			TTF_OpenFont(font_path, app->clocks[clock_index].rect_size);
		app->clocks[clock_index].fonts.mode = TTF_OpenFont(font_path, app->clocks[clock_index].rects.mode.h);
#ifdef _WIN32
		free(font_path);
#endif
	}
	if (app->clocks[clock_index].fonts.time == NULL || app->clocks[clock_index].fonts.mode == NULL) {
		fprintf(stderr, "%s\n", TTF_GetError());
		exit(EXIT_FAILURE);
	}
}

void flipclock_close_fonts(struct flipclock *app, int clock_index)
{
	if (app->clocks[clock_index].fonts.time != NULL)
		TTF_CloseFont(app->clocks[clock_index].fonts.time);
	if (app->clocks[clock_index].fonts.mode != NULL)
		TTF_CloseFont(app->clocks[clock_index].fonts.mode);
}

void flipclock_clear_texture(struct flipclock *app, int clock_index, SDL_Texture *target_texture,
			     SDL_Color background_color)
{
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, target_texture);
	SDL_SetRenderDrawColor(app->clocks[clock_index].renderer, background_color.r,
			       background_color.g, background_color.b,
			       background_color.a);
	SDL_RenderClear(app->clocks[clock_index].renderer);
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, NULL);
}

void flipclock_render_rounded_box(struct flipclock *app,
				  int clock_index,
				  SDL_Texture *target_texture,
				  SDL_Rect target_rect, int radius)
{
	if (radius <= 1) {
		SDL_SetRenderTarget(app->clocks[clock_index].renderer, target_texture);
		SDL_SetRenderDrawColor(app->clocks[clock_index].renderer, app->colors.rect.r,
				       app->colors.rect.g, app->colors.rect.b,
				       app->colors.rect.a);
		SDL_RenderFillRect(app->clocks[clock_index].renderer, &target_rect);
		SDL_SetRenderTarget(app->clocks[clock_index].renderer, NULL);
		return;
	}

	SDL_SetRenderTarget(app->clocks[clock_index].renderer, target_texture);
	SDL_SetRenderDrawColor(app->clocks[clock_index].renderer, app->colors.rect.r,
			       app->colors.rect.g, app->colors.rect.b,
			       app->colors.rect.a);
	if (2 * radius > target_rect.w)
		radius = target_rect.w / 2;
	if (2 * radius > target_rect.h)
		radius = target_rect.h / 2;
	int x = 0;
	int y = radius;
	int d = 3 - 2 * radius;
	while (x <= y) {
		SDL_RenderDrawLine(app->clocks[clock_index].renderer, target_rect.x + radius - x,
				   target_rect.y + radius - y,
				   target_rect.x + target_rect.w - radius + x -
					   1,
				   target_rect.y + radius - y);
		SDL_RenderDrawLine(app->clocks[clock_index].renderer, target_rect.x + radius - x,
				   target_rect.y + target_rect.h - radius + y,
				   target_rect.x + target_rect.w - radius + x -
					   1,
				   target_rect.y + target_rect.h - radius + y);
		SDL_RenderDrawLine(app->clocks[clock_index].renderer, target_rect.x + radius - y,
				   target_rect.y + radius - x,
				   target_rect.x + target_rect.w - radius + y -
					   1,
				   target_rect.y + radius - x);
		SDL_RenderDrawLine(app->clocks[clock_index].renderer, target_rect.x + radius - y,
				   target_rect.y + target_rect.h - radius + x,
				   target_rect.x + target_rect.w - radius + y -
					   1,
				   target_rect.y + target_rect.h - radius + x);
		if (d < 0) {
			d = d + 4 * x + 6;
		} else {
			d = d + 4 * (x - y) + 10;
			y--;
		}
		x++;
	}
	SDL_Rect temp_rect;
	temp_rect.x = target_rect.x;
	temp_rect.y = target_rect.y + radius;
	temp_rect.w = target_rect.w;
	temp_rect.h = target_rect.h - 2 * radius;
	SDL_RenderFillRect(app->clocks[clock_index].renderer, &temp_rect);
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, NULL);
}

void flipclock_render_text(struct flipclock *app, int clock_index, SDL_Texture *target_texture,
			   SDL_Rect target_rect, TTF_Font *font, char text[])
{
	int len = strlen(text);
	if (len > 2) {
		/* We can handle text longer than 2 chars, though. */
		fprintf(stderr, "Text length must be less than 3!");
		exit(EXIT_FAILURE);
	}
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, target_texture);
	for (int i = 0; i < len; i++) {
		/* We render text every minute, so we don't need cache. */
		SDL_Surface *text_surface = TTF_RenderGlyph_Shaded(
			font, text[i], app->colors.font, app->colors.rect);
		if (text_surface == NULL) {
			fprintf(stderr, "%s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		SDL_Texture *text_texture = SDL_CreateTextureFromSurface(
			app->clocks[clock_index].renderer, text_surface);
		if (text_texture == NULL) {
			fprintf(stderr, "%s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		SDL_Rect text_rect;
		text_rect.x = target_rect.x + target_rect.w / len * i +
			      (target_rect.w / len - text_surface->w) / 2;
		text_rect.y =
			target_rect.y + (target_rect.h - text_surface->h) / 2;
		text_rect.w = text_surface->w;
		text_rect.h = text_surface->h;
		SDL_FreeSurface(text_surface);
		SDL_RenderCopy(app->clocks[clock_index].renderer, text_texture, NULL, &text_rect);
		SDL_DestroyTexture(text_texture);
	}
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, NULL);
}

void flipclock_render_divider(struct flipclock *app, int clock_index,
			      SDL_Texture *target_texture, SDL_Rect target_rect)
{
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, target_texture);
	/* Don't be transparent, or you will not see divider. */
	SDL_SetRenderDrawColor(app->clocks[clock_index].renderer, app->colors.black.r,
			       app->colors.black.g, app->colors.black.b,
			       app->colors.black.a);
	SDL_RenderFillRect(app->clocks[clock_index].renderer, &target_rect);
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, NULL);
}

void flipclock_render_texture(struct flipclock *app, int clock_index)
{
	SDL_Texture *swap = app->clocks[clock_index].textures.current;
	app->clocks[clock_index].textures.current = app->clocks[clock_index].textures.previous;
	app->clocks[clock_index].textures.previous = swap;

	flipclock_clear_texture(app, clock_index, app->clocks[clock_index].textures.current,
				app->colors.transparent);

	char text[3];
	SDL_Rect divider_rect;

	/*
	 * Don't draw one card after another!
	 * I don't know why, but it seems SDL2 under Windows has a bug.
	 * If I draw text for one card, then draw background for another card.
	 * The background will be black. I've tested a lot and my code was
	 * correct. They just works under Linux! So I don't know why,
	 * but if I draw all backgrounds, then texts, and dividers,
	 * that is, in layer sequence, it just works.
	 */

	/* Background. */
	flipclock_render_rounded_box(app, clock_index, app->clocks[clock_index].textures.current,
				     app->clocks[clock_index].rects.hour, app->clocks[clock_index].radius);

	flipclock_render_rounded_box(app, clock_index, app->clocks[clock_index].textures.current,
				     app->clocks[clock_index].rects.minute, app->clocks[clock_index].radius);

	/* Text. */
	if (app->properties.ampm) {
		/* Just draw AM/PM text on hour card. */
		/*
		 * Don't use strftime() here,
		 * because font only have `A`, `P`, `M`.
		 */
		snprintf(text, sizeof(text), "%cM",
			 app->times.now.tm_hour / 12 ? 'P' : 'A');
		flipclock_render_text(app, clock_index, app->clocks[clock_index].textures.current,
				      app->clocks[clock_index].rects.mode, app->clocks[clock_index].fonts.mode, text);
	}

	/*
	 * MSVC does not support `%l` (` 1` - `12`),
	 * so we have to use `%I` (`01` - `12`), and trim zero.
	 */
	strftime(text, sizeof(text), app->properties.ampm ? "%I" : "%H",
		 &app->times.now);
	/* Trim zero when using 12-hour clock. */
	if (app->properties.ampm && text[0] == '0') {
		text[0] = text[1];
		text[1] = text[2];
	}
	flipclock_render_text(app, clock_index, app->clocks[clock_index].textures.current, app->clocks[clock_index].rects.hour,
			      app->clocks[clock_index].fonts.time, text);

	strftime(text, sizeof(text), "%M", &app->times.now);
	flipclock_render_text(app, clock_index, app->clocks[clock_index].textures.current, app->clocks[clock_index].rects.minute,
			      app->clocks[clock_index].fonts.time, text);

	/* And cut the card! */
	divider_rect.h = app->clocks[clock_index].rects.hour.h / 100;
	divider_rect.w = app->clocks[clock_index].rects.hour.w;
	divider_rect.x = app->clocks[clock_index].rects.hour.x;
	divider_rect.y =
		app->clocks[clock_index].rects.hour.y + (app->clocks[clock_index].rects.hour.h - divider_rect.h) / 2;
	flipclock_render_divider(app, clock_index, app->clocks[clock_index].textures.current, divider_rect);

	divider_rect.h = app->clocks[clock_index].rects.minute.h / 100;
	divider_rect.w = app->clocks[clock_index].rects.minute.w;
	divider_rect.x = app->clocks[clock_index].rects.minute.x;
	divider_rect.y = app->clocks[clock_index].rects.minute.y +
			 (app->clocks[clock_index].rects.minute.h - divider_rect.h) / 2;
	flipclock_render_divider(app, clock_index, app->clocks[clock_index].textures.current, divider_rect);
}

void flipclock_copy_rect(struct flipclock *app, int clock_index, SDL_Rect target_rect,
			 int progress)
{
	if (progress >= MAX_PROGRESS) {
		/* It finished flipping, so we don't draw flipping. */
		SDL_RenderCopy(app->clocks[clock_index].renderer, app->clocks[clock_index].textures.current,
			       &target_rect, &target_rect);
		return;
	}

	/* Draw the upper current digit and render it. */
	SDL_Rect half_source_rect;
	half_source_rect.x = target_rect.x;
	half_source_rect.y = target_rect.y;
	half_source_rect.w = target_rect.w;
	half_source_rect.h = target_rect.h / 2;
	SDL_Rect half_target_rect;
	half_target_rect.x = target_rect.x;
	half_target_rect.y = target_rect.y;
	half_target_rect.w = target_rect.w;
	half_target_rect.h = target_rect.h / 2;
	SDL_RenderCopy(app->clocks[clock_index].renderer, app->clocks[clock_index].textures.current, &half_source_rect,
		       &half_target_rect);

	/* Draw the lower previous digit and render it. */
	half_source_rect.x = target_rect.x;
	half_source_rect.y = target_rect.y + target_rect.h / 2;
	half_source_rect.w = target_rect.w;
	half_source_rect.h = target_rect.h / 2;
	half_target_rect.x = target_rect.x;
	half_target_rect.y = target_rect.y + target_rect.h / 2;
	half_target_rect.w = target_rect.w;
	half_target_rect.h = target_rect.h / 2;
	SDL_RenderCopy(app->clocks[clock_index].renderer, app->clocks[clock_index].textures.previous, &half_source_rect,
		       &half_target_rect);

	/*
	 * Draw the flip part.
	 * Upper half is previous and lower half is current.
	 * Just custom the destination Rect, zoom will be done automatically.
	 */
	bool upper_half = progress <= HALF_PROGRESS;
	double scale =
		upper_half ? 1.0 - (1.0 * progress) / HALF_PROGRESS :
			     ((1.0 * progress) - HALF_PROGRESS) / HALF_PROGRESS;
	half_source_rect.x = target_rect.x;
	half_source_rect.y =
		target_rect.y + (upper_half ? 0 : target_rect.h / 2);
	half_source_rect.w = target_rect.w;
	half_source_rect.h = target_rect.h / 2;
	half_target_rect.x = target_rect.x;
	half_target_rect.y =
		target_rect.y + (upper_half ? target_rect.h / 2 * (1 - scale) :
					      target_rect.h / 2);
	half_target_rect.w = target_rect.w;
	half_target_rect.h = target_rect.h / 2 * scale;
	SDL_RenderCopy(app->clocks[clock_index].renderer,
		       upper_half ? app->clocks[clock_index].textures.previous :
				    app->clocks[clock_index].textures.current,
		       &half_source_rect, &half_target_rect);
}

void flipclock_animate(struct flipclock *app, int clock_index, int progress)
{
	SDL_SetRenderDrawColor(app->clocks[clock_index].renderer, app->colors.black.r,
			       app->colors.black.g, app->colors.black.b,
			       app->colors.black.a);
	SDL_RenderClear(app->clocks[clock_index].renderer);
	flipclock_copy_rect(app, clock_index, app->clocks[clock_index].rects.hour,
			    app->times.now.tm_hour != app->times.past.tm_hour ?
				    progress :
				    MAX_PROGRESS);
	flipclock_copy_rect(app, clock_index, app->clocks[clock_index].rects.minute,
			    app->times.now.tm_min != app->times.past.tm_min ?
				    progress :
				    MAX_PROGRESS);
	SDL_RenderPresent(app->clocks[clock_index].renderer);
}

void flipclock_run_mainloop(struct flipclock *app)
{
	bool exit = false;
	bool animating = false;
	int progress = MAX_PROGRESS;
	unsigned int start_tick = SDL_GetTicks();
	SDL_Event event;
	/* Clear event queue before running. */
	while (SDL_PollEvent(&event))
		;
	/* First frame when app starts. */
	for (int i = 0; i < app->clocks_length; ++i) {
		flipclock_render_texture(app, i);
		flipclock_animate(app, i, MAX_PROGRESS);
	}
	while (!exit) {
#ifdef _WIN32
		/* Exit when preview window closed. */
		if (app->properties.preview &&
		    !IsWindow(app->properties.preview_window))
			exit = true;
#endif
		if (SDL_WaitEventTimeout(&event, 1000 / FPS)) {
			switch (event.type) {
#ifdef _WIN32
			/*
			 * There are a silly design in Windows' screensaver
			 * chooser. When you choose one screensaver, it will
			 * run the program with `/p HWND`, but if you changed
			 * to another, the former will not receive close
			 * event (yes, any kind of close event is not sent),
			 * and if you choose the former again, it will run
			 * the program with the same HWND again! To prevent
			 * this as a workaround, SDL sends
			 * SDL_RENDER_TARGETS_RESET when preview changes to
			 * another. I don't know why, but it works, and I
			 * don't know why no close event is sent.
			 */
			case SDL_RENDER_TARGETS_RESET:
				if (app->properties.preview)
					exit = true;
				break;
#endif
			case SDL_WINDOWEVENT:
				for (int i = 0; i < app->clocks_length; ++i) {
					if (event.window.windowID == SDL_GetWindowID(app->clocks[i].window)) {
						switch (event.window.event) {
						case SDL_WINDOWEVENT_SIZE_CHANGED:
							/*
							 * Only re-render when size changed.
							 * Windows may send event when size
							 * not changed, and cause strange bugs.
							 */
							if (event.window.data1 !=
								    app->clocks[i].width ||
							    event.window.data2 !=
								    app->clocks[i].height) {
								app->clocks[i].width =
									event.window.data1;
								app->clocks[i].height =
									event.window.data2;
								flipclock_destroy_textures(app, i);
								flipclock_close_fonts(app, i);
								flipclock_refresh(app, i);
								flipclock_open_fonts(app, i);
								flipclock_create_textures(app, i);
								flipclock_render_texture(app, i);
							}
							break;
						case SDL_WINDOWEVENT_MINIMIZED:
							app->clocks[i].wait = true;
							break;
						/* Dealing with EXPOSED to repaint. */
						case SDL_WINDOWEVENT_EXPOSED:
							app->clocks[i].wait = false;
							break;
						case SDL_WINDOWEVENT_CLOSE:
							exit = true;
							break;
						default:
							break;
						}
						break;
					}
				}
				break;
#ifdef _WIN32
			/*
			 * If under Windows, and not in preview window,
			 * and it was called as a screensaver,
			 * just exit when user press mouse button or move it,
			 * or interactive with touch screen.
			 */
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEWHEEL:
			case SDL_FINGERDOWN:
				if (!app->properties.preview &&
				    app->properties.screensaver)
					exit = true;
				break;
#endif
			case SDL_KEYDOWN:
#ifdef _WIN32
				/*
				 * If under Windows, and not in preview window,
				 * and it was called as a screensaver.
				 * just exit when user press any key.
				 * But if it was not called as a screensaver,
				 * it only handles some special keys.
				 * Also, we do nothing when in preview.
				 */
				if (!app->properties.preview &&
				    app->properties.screensaver) {
					exit = true;
				} else if (!app->properties.preview) {
					switch (event.key.keysym.sym) {
					case SDLK_ESCAPE:
					case SDLK_q:
						exit = true;
						break;
					case SDLK_t:
						app->properties.ampm =
							!app->properties.ampm;
						for (int i = 0; i < app->clocks_length; ++i)
							flipclock_render_texture(app, i);
						break;
					case SDLK_f:
						app->properties.full = !app->properties.full;
						for (int i = 0; i < app->clocks_length; ++i) {
							flipclock_destroy_textures(app, i);
							flipclock_close_fonts(app, i);
							flipclock_set_fullscreen(app, !app->properties.full);
							flipclock_refresh(app, i);
							flipclock_open_fonts(app, i);
							flipclock_create_textures(app, i);
							flipclock_render_texture(app, i);
						}
						break;
					default:
						break;
					}
				}
#else
				/* It's simple under Linux. */
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
				case SDLK_q:
					exit = true;
					break;
				case SDLK_t:
					app->properties.ampm =
						!app->properties.ampm;
					for (int i = 0; i < app->clocks_length; ++i)
						flipclock_render_texture(app, i);
					break;
				case SDLK_f:
					app->properties.full = !app->properties.full;
					for (int i = 0; i < app->clocks_length; ++i) {
						flipclock_destroy_textures(app, i);
						flipclock_close_fonts(app, i);
						flipclock_set_fullscreen(app, i, app->properties.full);
						flipclock_refresh(app, i);
						flipclock_open_fonts(app, i);
						flipclock_create_textures(app, i);
						flipclock_render_texture(app, i);
					}
					break;
				default:
					break;
				}
#endif
				break;
			case SDL_QUIT:
				exit = true;
				break;
			default:
				break;
			}
		}
		time_t raw_time = time(NULL);
		app->times.now = *localtime(&raw_time);
		/*
		 * Start animation when time changes.
		 * But don't sync time here!
		 * We need it to decide which part needs animation.
		 */
		if (!animating &&
		    (app->times.now.tm_hour != app->times.past.tm_hour ||
		     app->times.now.tm_min != app->times.past.tm_min)) {
			animating = true;
			progress = 0;
			start_tick = SDL_GetTicks();
			for (int i = 0; i < app->clocks_length; ++i)
				flipclock_render_texture(app, i);
		}
		/* Pause when minimized. */
		for (int i = 0; i < app->clocks_length; ++i) {
			if (!app->clocks[i].wait) {
				flipclock_animate(app, i, progress);
			}
		}
		/* Only calculate frame when animating. */
		if (animating)
			progress = SDL_GetTicks() - start_tick;
		/* Sync time when animation ends. */
		if (animating && progress > MAX_PROGRESS) {
			animating = false;
			progress = MAX_PROGRESS;
			app->times.past = app->times.now;
		}
	}
}

void flipclock_destroy_clocks(struct flipclock *app)
{
	for (int i = 0; i < app->clocks_length; ++i) {
		SDL_DestroyRenderer(app->clocks[i].renderer);
		SDL_DestroyWindow(app->clocks[i].window);
	}
	free(app->clocks);
	if (app->properties.full)
		SDL_ShowCursor(SDL_ENABLE);
#ifdef _WIN32
	if (!app->properties.screensaver)
		SDL_EnableScreenSaver();
#else
	SDL_EnableScreenSaver();
#endif
}

void flipclock_destroy(struct flipclock *app)
{
	if (app == NULL)
		return;
	free(app);
}

void flipclock_print_help(char program_name[])
{
	printf("A simple flip clock screensaver using SDL2.\n");
	printf("Usage: %s [OPTION...] <value>\n", program_name);
	printf("Options:\n");
	printf("\t%ch\t\tDisplay this help.\n", OPT_START);
#ifdef _WIN32
	printf("\t%cs\t\t(Windows only)"
	       "Required for starting screensaver in Windows.\n",
	       OPT_START);
	printf("\t%cc\t\t(Windows only)Dummy configuration.\n", OPT_START);
	printf("\t%cp <HWND>\t(Windows only)Preview in given window.\n",
	       OPT_START);
#endif
	printf("\t%cw\t\tRun in window, not fullscreen.\n", OPT_START);
	printf("\t%ct <12|24>\tToggle 12-hour clock format (AM/PM) "
	       "or 24-hour clock format.\n",
	       OPT_START);
	printf("\t%cf <font>\tCustom font path.\n", OPT_START);
	printf("Press `Esc` or `q` to exit.\n");
	printf("Press `f` to toggle fullscreen.\n");
	printf("Press `t` to toggle 12/24-hour clock format.\n");
}
