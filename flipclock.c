/*
 * Filename: flipclock.c
 * Created by 请叫我喵 Alynx.
 * alynx.zhou@gmail.com, http://alynx.xyz/.
 */
#include "flipclock.h"

bool init_app(struct app_all *app)
{
	/* Init SDL. */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "%s: SDL could not be inited! " \
			"SDL Error: %s\n", app->properties.program_name, \
			SDL_GetError());
		SDL_Quit();
		return false;
	}
	/* Calculate numbers. */
	if (app->properties.scale != 0.0) {
		app->properties.full = false;
		app->properties.width *= app->properties.scale;
		app->properties.height *= app->properties.scale;
	} else if (app->properties.full) {
		SDL_DisplayMode displayMode;
		SDL_GetCurrentDisplayMode(0, &displayMode);
		app->properties.width = displayMode.w;
		app->properties.height = displayMode.h;
		SDL_ShowCursor(SDL_DISABLE);
	}
	app->properties.rect_size = app->properties.width * 0.4;
 	app->properties.width_space = app->properties.width * 0.06;
	app->properties.time_radius = app->properties.rect_size / 10;
	app->rects.hour.x = (app->properties.width - 2 * \
			     app->properties.rect_size - \
			     app->properties.width_space) / 2;
	app->rects.hour.y = (app->properties.height - \
			     app->properties.rect_size) / 2;
	app->rects.hour.w = app->properties.rect_size;
	app->rects.hour.h = app->properties.rect_size;
	app->rects.minute.x = app->rects.hour.x + app->rects.hour.w + \
			      app->properties.width_space;
	app->rects.minute.y = app->rects.hour.y;
	app->rects.minute.w = app->properties.rect_size;
	app->rects.minute.h = app->properties.rect_size;
	app->rects.mode.w = app->properties.rect_size / 4;
	app->rects.mode.h = app->properties.rect_size / 8;
	app->rects.mode.x = (app->properties.width - app->rects.mode.w) / 2;
	app->rects.mode.y = (app->properties.height - \
			     app->properties.rect_size) / 2 + \
			    app->properties.rect_size + \
			    ((app->properties.height - \
			      app->properties.rect_size) / 2 - \
			     app->rects.mode.h) / 2;
	/* Create window. */
	app->window = SDL_CreateWindow(TITLE, \
				       SDL_WINDOWPOS_UNDEFINED, \
				       SDL_WINDOWPOS_UNDEFINED, \
				       app->properties.width, \
				       app->properties.height, \
				       (app->properties.full ? \
					SDL_WINDOW_FULLSCREEN_DESKTOP : \
					SDL_WINDOW_SHOWN) | \
				       SDL_WINDOW_ALLOW_HIGHDPI);
	if (app->window == NULL) {
		fprintf(stderr, "%s: Window could not be created! " \
			"SDL Error: %s\n", app->properties.program_name, \
			SDL_GetError());
		return false;
	}
	/* Create renderer. */
	app->renderer = SDL_CreateRenderer(app->window, -1, \
					   SDL_RENDERER_ACCELERATED | \
					   SDL_RENDERER_TARGETTEXTURE);
	if (app->renderer == NULL) {
		fprintf(stderr, "%s: Renderer could not be created! " \
			"SDL Error: %s\n", app->properties.program_name, \
			SDL_GetError());
		return false;
	}
	/* Main screen buffer texture. */
	app->textures.texture = SDL_CreateTexture(app->renderer, 0, \
						  SDL_TEXTUREACCESS_TARGET, \
						  app->properties.width, \
						  app->properties.height);
	if (app->textures.texture == NULL) {
		fprintf(stderr, "%s: Texture could not be created! " \
			"SDL Error: %s\n", app->properties.program_name, \
			SDL_GetError());
		return false;
	}
	/* Fill with black. */
	clear_texture(app, app->textures.texture, app->colors.black);
	/* Two transparent backend texture swap for tribuffer. */
	app->textures.current = SDL_CreateTexture(app->renderer, 0, \
						  SDL_TEXTUREACCESS_TARGET, \
						  app->properties.width, \
						  app->properties.height);
	if (app->textures.current == NULL) {
		fprintf(stderr, "%s: Texture could not be created! " \
			"SDL Error: %s\n", app->properties.program_name, \
			SDL_GetError());
		return false;
	}
	clear_texture(app, app->textures.current, app->colors.transparent);
	app->textures.previous = SDL_CreateTexture(app->renderer, 0, \
						   SDL_TEXTUREACCESS_TARGET, \
						   app->properties.width, \
						   app->properties.height);
	if (app->textures.previous == NULL) {
		fprintf(stderr, "%s: Texture could not be created! " \
			"SDL Error: %s\n", app->properties.program_name, \
			SDL_GetError());
		return false;
	}
	clear_texture(app, app->textures.previous, app->colors.transparent);
	/* Init SDL_ttf. */
	if (TTF_Init() < 0) {
		fprintf(stderr, "%s: SDL_ttf could not initialize! " \
			"SDL_ttf Error: %s\n", app->properties.program_name, \
			TTF_GetError());
		return false;
	}
	/* Load custom/fallback font. */
	if (app->properties.font_path != NULL) {
		app->fonts.time = TTF_OpenFont(app->properties.font_path, \
					       app->properties.rect_size);
		app->fonts.mode = TTF_OpenFont(app->properties.font_path, \
					       app->properties.rect_size / 10);
		if (app->fonts.time == NULL || app->fonts.mode == NULL) {
			fprintf(stderr, "%s: Custom font " \
				"could not be opened! " \
				"TTF Error: %s\n", \
				app->properties.program_name, TTF_GetError());
			app->properties.font_path = NULL;
		}
	}
	if (app->properties.font_path == NULL) {
		app->fonts.time = TTF_OpenFont(FALLBACK_FONT, \
					       app->properties.rect_size);
		app->fonts.mode = TTF_OpenFont(FALLBACK_FONT, \
					       app->properties.rect_size / 10);
		if (app->fonts.time == NULL || app->fonts.mode == NULL) {
			fprintf(stderr, "%s: Fallback font " \
				"could not be opened! " \
				"TTF Error: %s\n", \
				app->properties.program_name,  TTF_GetError());
			return false;
		}
	}
	return true;
}

void clear_texture(struct app_all *app, \
		   SDL_Texture *target_texture, \
		   SDL_Color background_color)
{
	SDL_SetRenderTarget(app->renderer, target_texture);
	SDL_SetRenderDrawColor(app->renderer, \
			       background_color.r, background_color.g, \
			       background_color.b, background_color.a);
	SDL_RenderClear(app->renderer);
}

void draw_rounded_box(struct app_all *app, \
		      SDL_Rect target_rect, \
		      int radius)
{
	if (radius <= 1) {
		SDL_RenderFillRect(app->renderer, &target_rect);
		return;
	}
	if (2 * radius > target_rect.w)
		radius = target_rect.w / 2;
	if (2 * radius > target_rect.h)
		radius = target_rect.h / 2;
	int x = 0;
	int y = radius;
	int d = 3 - 2 * radius;
	SDL_Rect temp_rect;
        while (x <= y) {
		SDL_RenderDrawLine(app->renderer, \
				   target_rect.x + radius - x, \
				   target_rect.y + radius - y, \
				   target_rect.x + target_rect.w - \
				   radius + x - 1, \
				   target_rect.y + radius - y);
		SDL_RenderDrawLine(app->renderer, \
				   target_rect.x + radius - x, \
				   target_rect.y + target_rect.h - \
				   radius + y, \
				   target_rect.x + target_rect.w - \
				   radius + x - 1, \
				   target_rect.y + target_rect.h - \
				   radius + y);
		SDL_RenderDrawLine(app->renderer, \
				   target_rect.x + radius - y, \
				   target_rect.y + radius - x, \
				   target_rect.x + target_rect.w - \
				   radius + y - 1, \
				   target_rect.y + radius - x);
		SDL_RenderDrawLine(app->renderer, \
				   target_rect.x + radius - y, \
				   target_rect.y + target_rect.h - \
				   radius + x, \
				   target_rect.x + target_rect.w - \
				   radius + y - 1, \
				   target_rect.y + target_rect.h - \
				   radius + x);
        	if (d < 0) {
			d = d + 4 * x + 6;
        	} else {
			d = d + 4 * (x - y) + 10;
        		y--;
        	}
        	x++;
        }
	temp_rect.x = target_rect.x;
	temp_rect.y = target_rect.y + radius;
	temp_rect.w = target_rect.w;
	temp_rect.h = target_rect.h - 2 * radius;
	SDL_RenderFillRect(app->renderer, &temp_rect);
}

void render_time(struct app_all *app, \
		 SDL_Texture *target_texture, \
		 SDL_Rect target_rect, \
		 TTF_Font *font, \
		 char digits[], \
		 int radius)
{
	SDL_Surface *text_surface = NULL;
	SDL_Texture *text_texture = NULL;
	SDL_Rect digit_rect;
	int i;
	if (strlen(digits) != 2) {
		fprintf(stderr, "%s: Wrong time digits!", \
			app->properties.program_name);
			return;
	}
	SDL_SetRenderTarget(app->renderer, target_texture);
	SDL_SetRenderDrawColor(app->renderer, \
			       app->colors.rect.r, app->colors.rect.g, \
			       app->colors.rect.b, app->colors.rect.a);
	draw_rounded_box(app, target_rect, radius);
	for (i = 0; i < 2; i++) {
		text_surface = TTF_RenderGlyph_Blended(font, digits[i], \
						       app->colors.font);
		if (text_surface == NULL) {
			fprintf(stderr, "%s: Unable to renderer " \
				"char into surface! " \
				"SDL Error: %s\n", \
				app->properties.program_name, SDL_GetError());
			SDL_FreeSurface(text_surface);
			return;
		}
		text_texture = SDL_CreateTextureFromSurface(app->renderer, \
							    text_surface);
		if (text_texture == NULL) {
			fprintf(stderr, "%s: Unable to renderer " \
				"surface into texture! " \
				"SDL Error: %s\n", \
				app->properties.program_name, SDL_GetError());
			SDL_DestroyTexture(text_texture);
			return;
		}
		digit_rect.x = target_rect.x + target_rect.w / 2 * i + \
			       (target_rect.w / 2 - text_surface->w) / 2;
		digit_rect.y = target_rect.y + \
			       (target_rect.h - \
				text_surface->h) / 2;
		digit_rect.w = text_surface->w;
		digit_rect.h = text_surface->h;
		SDL_FreeSurface(text_surface);
		SDL_SetRenderTarget(app->renderer, target_texture);
		SDL_RenderCopy(app->renderer, text_texture, NULL, &digit_rect);
		SDL_DestroyTexture(text_texture);
	}
}

void copy_frame(struct app_all *app, \
		SDL_Rect target_rect, \
		int step, \
		int max_steps)
{
	SDL_Rect half_source_rect, half_target_rect, divider_rect;
	double scale;
	/*
	 *Draw the upper current digit and render it.
	 * No need to render the previous lower digit.
	 * For it will remain.
	 */
	half_source_rect.x = target_rect.x;
	half_source_rect.y = target_rect.y;
	half_source_rect.w = target_rect.w;
	half_source_rect.h = target_rect.h / 2;
	half_target_rect.x = target_rect.x;
	half_target_rect.y = target_rect.y;
	half_target_rect.w = target_rect.w;
	half_target_rect.h = target_rect.h / 2;
	SDL_SetRenderTarget(app->renderer, app->textures.texture);
	SDL_RenderCopy(app->renderer, app->textures.current, \
		       &half_source_rect, &half_target_rect);
	/* Calculate the scale factor and the color change. */
	int half_steps = max_steps / 2;
	bool upper_half = step <= half_steps;
	scale = upper_half ? 1.0 - (1.0 * step) / half_steps : \
			     ((1.0 * step) - half_steps) / half_steps;
	/*
	 *Draw the flip. upper half is prev and lower half is current.
	 * Just custom the destination Rect, the Renderer will zoom
	 * automatically.
	 */
	half_source_rect.x = target_rect.x;
	half_source_rect.y = target_rect.y + \
			     (upper_half ? 0 : target_rect.h / 2);
	half_source_rect.w = target_rect.w;
	half_source_rect.h = target_rect.h / 2;
	half_target_rect.x = target_rect.x;
	half_target_rect.y = target_rect.y + (upper_half ? (target_rect.h * \
			     (1 - scale) / 2) : target_rect.h / 2);
	half_target_rect.w = target_rect.w;
	half_target_rect.h = target_rect.h * scale / 2;
	SDL_SetRenderTarget(app->renderer, app->textures.texture);
	SDL_RenderCopy(app->renderer, \
		       upper_half ? app->textures.previous : \
				    app->textures.current, \
		       &half_source_rect, &half_target_rect);
	/* render divider */
	divider_rect.h = target_rect.h == app->rects.mode.h ? \
			 target_rect.h / 40 : target_rect.h / 100;
	divider_rect.w = target_rect.w;
	divider_rect.x = target_rect.x;
	divider_rect.y = target_rect.y + (target_rect.h - divider_rect.h) / 2;
	SDL_SetRenderDrawColor(app->renderer, \
			       app->colors.transparent.r, \
			       app->colors.transparent.g, \
			       app->colors.transparent.b, \
			       app->colors.transparent.a);
	SDL_SetRenderTarget(app->renderer, app->textures.texture);
	SDL_RenderFillRect(app->renderer, &divider_rect);
	SDL_SetRenderTarget(app->renderer, NULL);
}

void animate_clock(struct app_all *app)
{
	/* Start tick. */
	int start_tick = SDL_GetTicks();
	int end_tick = start_tick + MAX_STEPS;
	int current_tick;
	int step;
	bool done = false;
	char now_digits[3];
	app->properties.mode_radius = app->rects.mode.w / 10;
	/* Update time. */
	if (app->times.past.tm_hour != app->times.now.tm_hour || \
	    app->times.past.tm_min != app->times.now.tm_min) {
		SDL_Texture *swap = app->textures.current;
		app->textures.current = app->textures.previous;
		app->textures.previous = swap;
	}
	if (app->properties.ampm && \
	    ((app->times.now.tm_hour / 12) != \
	     (app->times.past.tm_hour / 12))) {
		snprintf(now_digits, sizeof(now_digits), "%cM", \
			 app->times.now.tm_hour / 12 ? 'P' : 'A');
		render_time(app, app->textures.current, app->rects.mode, \
			    app->fonts.mode, now_digits, \
			    app->properties.mode_radius);
	}
	if (app->times.now.tm_hour != app->times.past.tm_hour) {
		app->properties.ampm ? \
		strftime(now_digits, sizeof(now_digits), \
			 "%I", &app->times.now) : \
		strftime(now_digits, sizeof(now_digits), \
			 "%H", &app->times.now);
		render_time(app, app->textures.current, app->rects.hour, \
			    app->fonts.time, now_digits, \
			    app->properties.time_radius);
	}
	if (app->times.now.tm_min != app->times.past.tm_min) {
		strftime(now_digits, sizeof(now_digits), \
			 "%M", &app->times.now);
		render_time(app, app->textures.current, app->rects.minute, \
			    app->fonts.time, now_digits, \
			    app->properties.time_radius);
	}
	while (!done) {
		current_tick = SDL_GetTicks();
		if (current_tick >= end_tick) {
			/* Align. */
			current_tick = end_tick;
			done = true;
		}
		step = MAX_STEPS * (current_tick - start_tick) / \
		       (end_tick - start_tick);
		if (app->properties.ampm && \
		    ((app->times.now.tm_hour / 12) != \
		     (app->times.past.tm_hour / 12)))
       			copy_frame(app, app->rects.mode, \
				   step, MAX_STEPS);
       		if (app->times.now.tm_hour != app->times.past.tm_hour)
       			copy_frame(app, app->rects.hour, \
				   step, MAX_STEPS);
       		if (app->times.now.tm_min != app->times.past.tm_min)
       			copy_frame(app, app->rects.minute, \
				   step, MAX_STEPS);
       		SDL_SetRenderTarget(app->renderer, NULL);
       		SDL_RenderCopy(app->renderer, app->textures.texture, \
			       NULL, NULL);
       		SDL_RenderPresent(app->renderer);
	}
	app->times.past = app->times.now;
}

Uint32 update_time(Uint32 interval, \
		   void *param)
{
	SDL_Event timer_event;
	time_t raw_time;
	struct app_all *app = (struct app_all *)param;
	raw_time = time(NULL);
	app->times.now = *localtime(&raw_time);
	if (app->times.now.tm_min != app->times.past.tm_min) {
		timer_event.type = SDL_USEREVENT;
		timer_event.user.code = 0;
		timer_event.user.data1 = NULL;
		timer_event.user.data2 = NULL;
		SDL_PushEvent(&timer_event);
		interval = 1000 * (60 - app->times.now.tm_sec) - 250;
	} else {
		interval = 250;
	}
	return interval;
}

void quit_app(struct app_all *app)
{
	TTF_CloseFont(app->fonts.time);
	TTF_CloseFont(app->fonts.mode);
	TTF_Quit();
	SDL_DestroyTexture(app->textures.previous);
	SDL_DestroyTexture(app->textures.current);
	SDL_DestroyTexture(app->textures.texture);
	SDL_DestroyRenderer(app->renderer);
	if (app->properties.full)
		SDL_ShowCursor(SDL_ENABLE);
	SDL_DestroyWindow(app->window);
	SDL_Quit();
}

void print_help(char *program_name)
{
	printf("A simple flip clock screensaver using SDL2.\n");
	printf("Written by AlynxZhou. Version %s.\n", VERSION);
	printf("Usage: %s [OPTION...] <value>\n", program_name);
	printf("Options:\n");
	printf("\t-h\t\tDisplay this help.\n");
	printf("\t-w\t\tRun in window.\n");
	printf("\t-t <12|24>\tToggle 12-hour clock format (AM/PM) " \
	       "or 24-hour clock format.\n");
	printf("\t-f <font>\tCustom font.\n");
	printf("\t-s <factor>\tCustom resolution with a scale factor.\n");
	printf("Press `q` or `Esc` to quit.\n");
}
