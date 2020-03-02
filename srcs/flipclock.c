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
#define TITLE "FlipClock"

struct flipclock *flipclock_create(void)
{
	struct flipclock *app = malloc(sizeof(*app));
	if (app == NULL) {
		fprintf(stderr, "Failed to create app!\n");
		exit(EXIT_FAILURE);
	}
	app->window = NULL;
	app->renderer = NULL;
	app->textures.current = NULL;
	app->textures.previous = NULL;
	app->fonts.time = NULL;
	app->fonts.mode = NULL;
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
	app->properties.width = WINDOW_WIDTH;
	app->properties.height = WINDOW_HEIGHT;
	app->properties.ampm = false;
	app->properties.full = true;
	app->properties.font_path = NULL;
#ifdef _WIN32
	app->properties.preview = false;
#endif
	time_t raw_time = time(NULL);
	app->times.past = *localtime(&raw_time);
	app->times.now = *localtime(&raw_time);
	return app;
}

void flipclock_create_window(struct flipclock *app)
{
	SDL_DisableScreenSaver();
#ifdef _WIN32
	if (app->properties.preview) {
		/* Don't set fullscreen if in preview. */
		app->properties.full = false;
		/* Create window from native window when in preview. */
		app->window =
			SDL_CreateWindowFrom(app->properties.preview_window);
	} else {
		unsigned int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
				     SDL_WINDOW_ALLOW_HIGHDPI;
		if (app->properties.full) {
			flags = SDL_WINDOW_SHOWN |
				SDL_WINDOW_FULLSCREEN_DESKTOP |
				SDL_WINDOW_ALLOW_HIGHDPI;
			SDL_ShowCursor(SDL_DISABLE);
		}
		app->window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED,
					       SDL_WINDOWPOS_UNDEFINED,
					       app->properties.width,
					       app->properties.height, flags);
	}
#else
	unsigned int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
			     SDL_WINDOW_ALLOW_HIGHDPI;
	if (app->properties.full) {
		flags = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP |
			SDL_WINDOW_ALLOW_HIGHDPI;
		SDL_ShowCursor(SDL_DISABLE);
	}
	app->window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED,
				       SDL_WINDOWPOS_UNDEFINED,
				       app->properties.width,
				       app->properties.height, flags);
#endif
	if (app->window == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	/* Init window size after create it. */
	SDL_GetWindowSize(app->window, &(app->properties.width),
			  &(app->properties.height));
	app->renderer = SDL_CreateRenderer(app->window, -1,
					   SDL_RENDERER_ACCELERATED |
						   SDL_RENDERER_TARGETTEXTURE |
						   SDL_RENDERER_PRESENTVSYNC);
	if (app->renderer == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
}

void flipclock_set_fullscreen(struct flipclock *app, bool full)
{
	app->properties.full = full;
	if (full) {
		SDL_SetWindowFullscreen(app->window,
					SDL_WINDOW_FULLSCREEN_DESKTOP);
		SDL_GetWindowSize(app->window, &app->properties.width,
				  &app->properties.height);
		SDL_ShowCursor(SDL_DISABLE);
	} else {
		SDL_SetWindowFullscreen(app->window, 0);
		SDL_GetWindowSize(app->window, &app->properties.width,
				  &app->properties.height);
		SDL_ShowCursor(SDL_ENABLE);
	}
}

void flipclock_refresh(struct flipclock *app)
{
	if (app->properties.width < app->properties.height) {
		/* Some user do love portrait. */
		app->properties.rect_size =
			app->properties.height * 0.4 >
					app->properties.width * 0.8 ?
				app->properties.width * 0.8 :
				app->properties.height * 0.4;
		int space = app->properties.height * 0.06;
		app->properties.radius = app->properties.rect_size / 10;

		app->rects.hour.y = (app->properties.height -
				     2 * app->properties.rect_size - space) /
				    2;
		app->rects.hour.x =
			(app->properties.width - app->properties.rect_size) / 2;
		app->rects.hour.w = app->properties.rect_size;
		app->rects.hour.h = app->properties.rect_size;

		app->rects.minute.y =
			app->rects.hour.y + app->rects.hour.h + space;
		app->rects.minute.x = app->rects.hour.x;
		app->rects.minute.w = app->properties.rect_size;
		app->rects.minute.h = app->properties.rect_size;
	} else {
		/* But others love landscape. */
		app->properties.rect_size =
			app->properties.width * 0.4 >
					app->properties.height * 0.8 ?
				app->properties.height * 0.8 :
				app->properties.width * 0.4;
		int space = app->properties.width * 0.06;
		app->properties.radius = app->properties.rect_size / 10;

		app->rects.hour.x = (app->properties.width -
				     2 * app->properties.rect_size - space) /
				    2;
		app->rects.hour.y =
			(app->properties.height - app->properties.rect_size) /
			2;
		app->rects.hour.w = app->properties.rect_size;
		app->rects.hour.h = app->properties.rect_size;

		app->rects.minute.x =
			app->rects.hour.x + app->rects.hour.w + space;
		app->rects.minute.y = app->rects.hour.y;
		app->rects.minute.w = app->properties.rect_size;
		app->rects.minute.h = app->properties.rect_size;
	}

	/* How do I get those numbers? Test. */
	app->rects.mode.w = app->properties.rect_size / 5;
	app->rects.mode.h = app->properties.rect_size / 10;
	app->rects.mode.x = app->rects.hour.x + app->properties.rect_size / 50;
	app->rects.mode.y = app->rects.hour.y + app->properties.rect_size -
			    app->rects.mode.h - app->properties.rect_size / 35;
}

void flipclock_create_textures(struct flipclock *app)
{
	/* Two transparent backend texture swap for tribuffer. */
	app->textures.current = SDL_CreateTexture(app->renderer, 0,
						  SDL_TEXTUREACCESS_TARGET,
						  app->properties.width,
						  app->properties.height);
	if (app->textures.current == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetTextureBlendMode(app->textures.current, SDL_BLENDMODE_BLEND);
	app->textures.previous = SDL_CreateTexture(app->renderer, 0,
						   SDL_TEXTUREACCESS_TARGET,
						   app->properties.width,
						   app->properties.height);
	if (app->textures.previous == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetTextureBlendMode(app->textures.previous, SDL_BLENDMODE_BLEND);
}

void flipclock_destroy_textures(struct flipclock *app)
{
	if (app->textures.current != NULL)
		SDL_DestroyTexture(app->textures.current);
	if (app->textures.previous != NULL)
		SDL_DestroyTexture(app->textures.previous);
}

void flipclock_open_fonts(struct flipclock *app)
{
	if (app->properties.font_path != NULL) {
		app->fonts.time = TTF_OpenFont(app->properties.font_path,
					       app->properties.rect_size);
		app->fonts.mode = TTF_OpenFont(app->properties.font_path,
					       app->rects.mode.h);
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
		app->fonts.time =
			TTF_OpenFont(font_path, app->properties.rect_size);
		app->fonts.mode = TTF_OpenFont(font_path, app->rects.mode.h);
#ifdef _WIN32
		free(font_path);
#endif
	}
	if (app->fonts.time == NULL || app->fonts.mode == NULL) {
		fprintf(stderr, "%s\n", TTF_GetError());
		exit(EXIT_FAILURE);
	}
}

void flipclock_close_fonts(struct flipclock *app)
{
	if (app->fonts.time != NULL)
		TTF_CloseFont(app->fonts.time);
	if (app->fonts.mode != NULL)
		TTF_CloseFont(app->fonts.mode);
}

void flipclock_clear_texture(struct flipclock *app, SDL_Texture *target_texture,
			     SDL_Color background_color)
{
	SDL_SetRenderTarget(app->renderer, target_texture);
	SDL_SetRenderDrawColor(app->renderer, background_color.r,
			       background_color.g, background_color.b,
			       background_color.a);
	SDL_RenderClear(app->renderer);
	SDL_SetRenderTarget(app->renderer, NULL);
}

void flipclock_render_rounded_box(struct flipclock *app,
				  SDL_Texture *target_texture,
				  SDL_Rect target_rect, int radius)
{
	if (radius <= 1) {
		SDL_SetRenderTarget(app->renderer, target_texture);
		SDL_SetRenderDrawColor(app->renderer, app->colors.rect.r,
				       app->colors.rect.g, app->colors.rect.b,
				       app->colors.rect.a);
		SDL_RenderFillRect(app->renderer, &target_rect);
		SDL_SetRenderTarget(app->renderer, NULL);
		return;
	}

	SDL_SetRenderTarget(app->renderer, target_texture);
	SDL_SetRenderDrawColor(app->renderer, app->colors.rect.r,
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
		SDL_RenderDrawLine(app->renderer, target_rect.x + radius - x,
				   target_rect.y + radius - y,
				   target_rect.x + target_rect.w - radius + x -
					   1,
				   target_rect.y + radius - y);
		SDL_RenderDrawLine(app->renderer, target_rect.x + radius - x,
				   target_rect.y + target_rect.h - radius + y,
				   target_rect.x + target_rect.w - radius + x -
					   1,
				   target_rect.y + target_rect.h - radius + y);
		SDL_RenderDrawLine(app->renderer, target_rect.x + radius - y,
				   target_rect.y + radius - x,
				   target_rect.x + target_rect.w - radius + y -
					   1,
				   target_rect.y + radius - x);
		SDL_RenderDrawLine(app->renderer, target_rect.x + radius - y,
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
	SDL_RenderFillRect(app->renderer, &temp_rect);
	SDL_SetRenderTarget(app->renderer, NULL);
}

void flipclock_render_text(struct flipclock *app, SDL_Texture *target_texture,
			   SDL_Rect target_rect, TTF_Font *font, char text[])
{
	int len = strlen(text);
	if (len > 2) {
		/* We can handle text longer than 2 chars, though. */
		fprintf(stderr, "Text length must be less than 3!");
		exit(EXIT_FAILURE);
	}
	SDL_SetRenderTarget(app->renderer, target_texture);
	for (int i = 0; i < len; i++) {
		SDL_Surface *text_surface = TTF_RenderGlyph_Shaded(
			font, text[i], app->colors.font, app->colors.rect);
		if (text_surface == NULL) {
			fprintf(stderr, "%s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		SDL_Texture *text_texture = SDL_CreateTextureFromSurface(
			app->renderer, text_surface);
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
		SDL_RenderCopy(app->renderer, text_texture, NULL, &text_rect);
		SDL_DestroyTexture(text_texture);
	}
	SDL_SetRenderTarget(app->renderer, NULL);
}

void flipclock_render_divider(struct flipclock *app,
			      SDL_Texture *target_texture, SDL_Rect target_rect)
{
	SDL_SetRenderTarget(app->renderer, target_texture);
	/* Don't be transparent, or you will not see divider. */
	SDL_SetRenderDrawColor(app->renderer, app->colors.black.r,
			       app->colors.black.g, app->colors.black.b,
			       app->colors.black.a);
	SDL_RenderFillRect(app->renderer, &target_rect);
	SDL_SetRenderTarget(app->renderer, NULL);
}

void flipclock_render_texture(struct flipclock *app)
{
	SDL_Texture *swap = app->textures.current;
	app->textures.current = app->textures.previous;
	app->textures.previous = swap;

	flipclock_clear_texture(app, app->textures.current,
				app->colors.transparent);

	char text[3];
	SDL_Rect divider_rect;

	/* Let's draw hour! */
	/* Background. */
	flipclock_render_rounded_box(app, app->textures.current,
				     app->rects.hour, app->properties.radius);
	/* Text. */
	strftime(text, sizeof(text), app->properties.ampm ? "%l" : "%H",
		 &app->times.now);
	/* Trim space when using 12-hour clock. */
	if (isspace(text[0])) {
		text[0] = text[1];
		text[1] = text[2];
	}
	flipclock_render_text(app, app->textures.current, app->rects.hour,
			      app->fonts.time, text);
	/* And cut the card! */
	divider_rect.h = app->rects.hour.h / 100;
	divider_rect.w = app->rects.hour.w;
	divider_rect.x = app->rects.hour.x;
	divider_rect.y =
		app->rects.hour.y + (app->rects.hour.h - divider_rect.h) / 2;
	flipclock_render_divider(app, app->textures.current, divider_rect);

	/* Let's draw minute! */
	/* Background. */
	flipclock_render_rounded_box(app, app->textures.current,
				     app->rects.minute, app->properties.radius);
	/* Text. */
	strftime(text, sizeof(text), "%M", &app->times.now);
	flipclock_render_text(app, app->textures.current, app->rects.minute,
			      app->fonts.time, text);
	/* And cut the card! */
	divider_rect.h = app->rects.minute.h / 100;
	divider_rect.w = app->rects.minute.w;
	divider_rect.x = app->rects.minute.x;
	divider_rect.y = app->rects.minute.y +
			 (app->rects.minute.h - divider_rect.h) / 2;
	flipclock_render_divider(app, app->textures.current, divider_rect);

	if (app->properties.ampm) {
		/* Just draw AM/PM text on hour card. */
		/*
		 * Don't use strftime() here,
		 * because font only have `A`, `P`, `M`.
		 */
		snprintf(text, sizeof(text), "%cM",
			 app->times.now.tm_hour / 12 ? 'P' : 'A');
		flipclock_render_text(app, app->textures.current,
				      app->rects.mode, app->fonts.mode, text);
	}
}

void flipclock_copy_rect(struct flipclock *app, SDL_Rect target_rect,
			 int progress)
{
	if (progress >= MAX_PROGRESS) {
		/* It finished flipping, so we don't draw flipping. */
		SDL_RenderCopy(app->renderer, app->textures.current,
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
	SDL_RenderCopy(app->renderer, app->textures.current, &half_source_rect,
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
	SDL_RenderCopy(app->renderer, app->textures.previous, &half_source_rect,
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
	SDL_RenderCopy(app->renderer,
		       upper_half ? app->textures.previous :
				    app->textures.current,
		       &half_source_rect, &half_target_rect);
}

void flipclock_animate(struct flipclock *app, int progress)
{
	SDL_SetRenderDrawColor(app->renderer, app->colors.black.r,
			       app->colors.black.g, app->colors.black.b,
			       app->colors.black.a);
	SDL_RenderClear(app->renderer);
	flipclock_copy_rect(app, app->rects.hour,
			    app->times.now.tm_hour != app->times.past.tm_hour ?
				    progress :
				    MAX_PROGRESS);
	flipclock_copy_rect(app, app->rects.minute,
			    app->times.now.tm_min != app->times.past.tm_min ?
				    progress :
				    MAX_PROGRESS);
	SDL_RenderPresent(app->renderer);
}

void flipclock_run_mainloop(struct flipclock *app)
{
	bool exit = false;
	bool wait = false;
	bool animating = false;
	int progress = MAX_PROGRESS;
	unsigned int start_tick = SDL_GetTicks();
	SDL_Event event;
	/* First frame when app starts. */
	flipclock_render_texture(app);
	flipclock_animate(app, MAX_PROGRESS);
	while (!exit) {
#ifdef _WIN32
		/* Exit when preview window closed. */
		if (app->properties.preview &&
		    !IsWindow(app->properties.preview_window))
			exit = true;
#endif
		if (SDL_WaitEventTimeout(&event, 1000 / FPS)) {
			switch (event.type) {
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					/*
					 * Only re-render when size changed.
					 * Windows may send event when size
					 * not changed, and cause strange bugs.
					 */
					if (event.window.data1 !=
						    app->properties.width ||
					    event.window.data2 !=
						    app->properties.height) {
						app->properties.width =
							event.window.data1;
						app->properties.height =
							event.window.data2;
						flipclock_destroy_textures(app);
						flipclock_close_fonts(app);
						flipclock_refresh(app);
						flipclock_open_fonts(app);
						flipclock_create_textures(app);
						flipclock_render_texture(app);
					}
					break;
				case SDL_WINDOWEVENT_MINIMIZED:
					wait = true;
					break;
				/* Dealing with EXPOSED to repaint. */
				case SDL_WINDOWEVENT_EXPOSED:
					wait = false;
					break;
				case SDL_WINDOWEVENT_CLOSE:
					exit = true;
					break;
				default:
					break;
				}
				break;
			case SDL_KEYDOWN:
			case SDL_MOUSEBUTTONDOWN:
#ifdef _WIN32
				if (!app->properties.preview)
					exit = true;
#else
				exit = true;
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
			flipclock_render_texture(app);
		}
		/* Pause when minimized. */
		if (!wait)
			flipclock_animate(app, progress);
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

void flipclock_destroy_window(struct flipclock *app)
{
	SDL_DestroyRenderer(app->renderer);
	SDL_DestroyWindow(app->window);
	if (app->properties.full)
		SDL_ShowCursor(SDL_ENABLE);
	SDL_EnableScreenSaver();
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
	printf("Press any key to exit.\n");
}
