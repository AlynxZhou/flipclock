/**
 * Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.one/)
 */
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "getarg.h"
#include "flipclock.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FPS 60
#define MAX_PROGRESS 300
#define HALF_PROGRESS (MAX_PROGRESS / 2)
#define DOUBLE_TAP_INTERVAL_MS 300

#if defined(_WIN32)
void _flipclock_get_program_dir_win32(char *program_dir)
{
	/**
	 * See https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew.
	 * GetModuleFileName is a macro to
	 * GetModuleFileNameW and GetModuleFileNameA.
	 */
	GetModuleFileName(NULL, program_dir, MAX_BUFFER_LENGTH);
	program_dir[MAX_BUFFER_LENGTH - 1] = '\0';
	if (strlen(program_dir) == MAX_BUFFER_LENGTH - 1)
		LOG_ERROR("program_dir too long, may fail to load.\n");
	/**
	 * Remove the program name and get the dir.
	 * This should work because Windows path should have `\\`.
	 */
	for (int i = strlen(program_dir) - 1; i >= 0; --i) {
		if (program_dir[i] == '\\') {
			program_dir[i] = '\0';
			break;
		}
	}
}

void _flipclock_get_conf_path_win32(char *conf_path, const char *program_dir)
{
	snprintf(conf_path, MAX_BUFFER_LENGTH, "%s\\flipclock.conf",
		 program_dir);
	conf_path[MAX_BUFFER_LENGTH - 1] = '\0';
	if (strlen(conf_path) == MAX_BUFFER_LENGTH - 1)
		LOG_ERROR("conf_path too long, may fail to load.\n");
}
#elif defined(__linux__)
void _flipclock_get_conf_path_linux(char *conf_path)
{
	// Be a good program.
	const char *conf_dir = getenv("XDG_CONFIG_HOME");
	if (conf_dir == NULL || strlen(conf_dir) == 0) {
		// Linux users should not be homeless.
		const char *home = getenv("HOME");
		snprintf(conf_path, MAX_BUFFER_LENGTH,
			 "%s/.config/flipclock.conf", home);
	} else {
		snprintf(conf_path, MAX_BUFFER_LENGTH, "%s/flipclock.conf",
			 conf_dir);
	}
	conf_path[MAX_BUFFER_LENGTH - 1] = '\0';
	if (strlen(conf_path) == MAX_BUFFER_LENGTH - 1)
		LOG_ERROR("conf_path too long, may fail to load.\n");
}
#endif

struct flipclock *flipclock_create(void)
{
	struct flipclock *app = malloc(sizeof(*app));
	if (app == NULL) {
		LOG_ERROR("Failed to create app!\n");
		exit(EXIT_FAILURE);
	}
	app->clocks = NULL;
	// Should create 1 clock in windowed mode.
	app->clocks_length = 1;
	app->last_touch = 0;
	app->running = true;
	app->colors.black.r = 0x00;
	app->colors.black.g = 0x00;
	app->colors.black.b = 0x00;
	app->colors.black.a = 0xff;
	app->colors.transparent.r = 0x00;
	app->colors.transparent.g = 0x00;
	app->colors.transparent.b = 0x00;
	app->colors.transparent.a = 0x00;
	app->colors.font.r = 0xd0;
	app->colors.font.g = 0xd0;
	app->colors.font.b = 0xd0;
	app->colors.font.a = 0xff;
	app->colors.rect.r = 0x20;
	app->colors.rect.g = 0x20;
	app->colors.rect.b = 0x20;
	app->colors.rect.a = 0xff;
	app->colors.back = app->colors.black;
	app->properties.ampm = false;
	app->properties.full = true;
	app->properties.font_path[0] = '\0';
	app->properties.font_scale = 1.0;
	app->properties.rect_scale = 1.0;
#ifdef _WIN32
	app->properties.preview = false;
	app->properties.screensaver = false;
	app->properties.program_dir[0] = '\0';
	_flipclock_get_program_dir_win32(app->properties.program_dir);
	LOG_DEBUG("Using program_dir `%s`.\n", app->properties.program_dir);
#endif
#if defined(_WIN32)
	_flipclock_get_conf_path_win32(app->properties.conf_path,
				       app->properties.program_dir);
#elif defined(__linux__)
	_flipclock_get_conf_path_linux(app->properties.conf_path);
#endif
#ifndef __ANDROID__
	LOG_DEBUG("Using conf_path `%s`.\n", app->properties.conf_path);
#endif
	time_t raw_time = time(NULL);
	app->times.past = *localtime(&raw_time);
	app->times.now = *localtime(&raw_time);
	return app;
}

int _flipclock_parse_key_value(char *line, char **key, char **value)
{
	const int line_length = strlen(line);
	int key_start = -1;
	for (int i = 0; i < line_length; ++i) {
		if (!isspace(line[i])) {
			key_start = i;
			break;
		}
	}
	if (key_start == -1) {
		LOG_DEBUG("No `key_start` found! Skip empty line.\n");
		return -1;
	}
	if (line[key_start] == '#' || line[key_start] == ';') {
		LOG_DEBUG("Skip comment line.\n");
		return 1;
	}
	int value_end = -1;
	for (int i = line_length - 1; i >= 0; --i) {
		if (!isspace(line[i])) {
			value_end = i + 1;
			break;
		}
	}
	if (value_end == -1) {
		LOG_DEBUG("No `value_end` found! Skip empty line.\n");
		return -2;
	}
	line[value_end] = '\0';
	int equal = -1;
	for (int i = 0; i < line_length; ++i) {
		if (line[i] == '=') {
			equal = i;
			break;
		}
	}
	if (equal == -1) {
		LOG_ERROR("No `=` found! Invalid line!\n");
		return -3;
	}
	int key_end = -1;
	for (int i = equal - 1; i >= key_start; --i) {
		if (!isspace(line[i])) {
			key_end = i + 1;
			break;
		}
	}
	if (key_end == -1) {
		LOG_ERROR("No `key_end` found! Invalid line!\n");
		return -4;
	}
	line[key_end] = '\0';
	int value_start = -1;
	for (int i = equal + 1; i < value_end; ++i) {
		if (!isspace(line[i])) {
			value_start = i;
			break;
		}
	}
	if (value_start == -1) {
		LOG_ERROR("No `value_start` found! Invalid line!\n");
		return -5;
	}
	*key = line + key_start;
	*value = line + value_start;
	return 0;
}

int _flipclock_parse_color(const char *rgba, SDL_Color *color)
{
	const int rgba_length = strlen(rgba);
	if (rgba_length == 0) {
		LOG_ERROR("Empty color string!\n");
		return -1;
	} else if (rgba[0] != '#') {
		LOG_ERROR("Color string must start with `#`!\n");
		return -2;
	} else if (rgba_length != 7 && rgba_length != 9) {
		LOG_ERROR("Color string must be in format `#rrggbb[aa]`!\n");
		return -3;
	} else {
		for (int i = 1; i < rgba_length; ++i) {
			// Cool, ctype.h always gives me surprise.
			if (!isxdigit(rgba[i])) {
				LOG_ERROR("Color string numbers "
					  "should be hexcode!\n");
				return -4;
			}
		}
		/**
		 * Even if user input an invalid hexcode,
		 * we also let strtoll try to parse it.
		 * It's user's problem when displayed color
		 * is not what he/she wants.
		 */
		long long hex_number = strtoll(rgba + 1, NULL, 16);
		// Add 0xff as alpha if no alpha provided.
		if (rgba_length == 7)
			hex_number = (hex_number << 8) | 0xff;
		color->r = (hex_number >> 24) & 0xff;
		color->g = (hex_number >> 16) & 0xff;
		color->b = (hex_number >> 8) & 0xff;
		color->a = (hex_number >> 0) & 0xff;
		LOG_DEBUG("Parsed color `rgba(%d, %d, %d, %d)`.\n", color->r,
			  color->g, color->b, color->a);
	}
	return 0;
}

void flipclock_load_conf(struct flipclock *app)
{
	FILE *conf = fopen(app->properties.conf_path, "r");
	/**
	 * See https://docs.microsoft.com/en-us/cpp/c-runtime-library/errno-constants?view=msvc-160.
	 * errno seems work on MSVC.
	 */
	if (conf == NULL && errno == ENOENT) {
		LOG_DEBUG("File not found, create with default content.\n");
		conf = fopen(app->properties.conf_path, "w");
		if (conf == NULL) {
			// Just skip conf, it's able to run.
			LOG_ERROR("Failed to write default content!\n");
			goto out;
		}
		fputs("# Uncomment `ampm = true` to use 12-hour format.\n"
		      "#ampm = true\n"
		      "# Uncomment `full = false` to disable fullscreen.\n"
		      "# You should not change this for a screensaver.\n"
		      "#full = false\n"
		      "# Uncomment `font = ` and "
		      "add path to use custom font.\n"
		      "#font = \n"
		      "# Uncomment `font_scale = 0.8` to modify digit scale.\n"
		      "# This scales the digits again based on card scale.\n"
		      "#font_scale = 0.8\n"
		      "# Uncomment `rect_scale = 0.8` to modify card scale.\n"
		      "# This also scales the digits.\n"
		      "#rect_scale = 0.8\n"
		      "# Uncomment `font_color = ` to modify "
		      "the color of the digit.\n"
		      "#font_color = #000000ff\n"
		      "# Uncomment `rect_color = ` to modify "
		      "the color of the card.\n"
		      "#rect_color = #fe9a00ff\n"
		      "# Uncomment `black_color = ` to modify "
		      "the color of the background.\n"
		      "#back_color = #000000ff\n",
		      conf);
		goto close_file;
	}
	/**
	 * Most file systems have max file name length limit.
	 * So I don't need to alloc memory dynamically.
	 */
	char conf_line[MAX_BUFFER_LENGTH];
	char *key;
	char *value;
	/**
	 * It's better to use a temp variable here,
	 * so when parsing failed we still have the default color.
	 */
	SDL_Color parsed_color;
	while (fgets(conf_line, MAX_BUFFER_LENGTH, conf) != NULL) {
		if (strlen(conf_line) == MAX_BUFFER_LENGTH - 1)
			LOG_ERROR("conf_line too long, may fail to load.\n");
		if (_flipclock_parse_key_value(conf_line, &key, &value))
			continue;
		LOG_DEBUG("Parsed key `%s` and value `%s`.\n", key, value);
		if (!strcmp(key, "ampm")) {
			if (!strcmp(value, "true"))
				app->properties.ampm = true;
		} else if (!strcmp(key, "full")) {
			if (!strcmp(value, "false"))
				app->properties.full = false;
		} else if (!strcmp(key, "font")) {
			strncpy(app->properties.font_path, value,
				MAX_BUFFER_LENGTH - 1);
			app->properties.font_path[MAX_BUFFER_LENGTH - 1] = '\0';
			if (strlen(app->properties.font_path) ==
			    MAX_BUFFER_LENGTH - 1)
				LOG_ERROR("font_path too long, "
					  "may fail to load.\n");
		} else if (!strcmp(key, "font_scale")) {
			app->properties.font_scale = strtod(value, NULL);
		} else if (!strcmp(key, "rect_scale")) {
			app->properties.rect_scale = strtod(value, NULL);
		} else if (!strcmp(key, "font_color")) {
			if (!_flipclock_parse_color(value, &parsed_color))
				app->colors.font = parsed_color;
			else
				LOG_ERROR("Failed to parse font_color!\n");
		} else if (!strcmp(key, "rect_color")) {
			if (!_flipclock_parse_color(value, &parsed_color))
				app->colors.rect = parsed_color;
			else
				LOG_ERROR("Failed to parse rect_color!\n");
		} else if (!strcmp(key, "back_color")) {
			if (!_flipclock_parse_color(value, &parsed_color))
				app->colors.back = parsed_color;
			else
				LOG_ERROR("Failed to parse back_color!\n");
		} else {
			LOG_DEBUG("Unknown key `%s`.\n", key);
		}
	}
close_file:
	fclose(conf);
out:
	return;
}

void _flipclock_create_clocks_default(struct flipclock *app)
{
	/**
	 * We need `SDL_WINDOW_RESIZABLE` for auto-rotate
	 * while fullscreen on Android.
	 */
	unsigned int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
			     SDL_WINDOW_ALLOW_HIGHDPI;
	// Create window for each display if fullscreen.
	if (app->properties.full) {
		/**
		 * Instead of handling display number changing,
		 * let user restart program is easier.
		 */
		app->clocks_length = SDL_GetNumVideoDisplays();
		flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
			SDL_WINDOW_ALLOW_HIGHDPI |
			SDL_WINDOW_FULLSCREEN_DESKTOP;
		SDL_ShowCursor(SDL_DISABLE);
	}
	app->clocks = malloc(sizeof(*app->clocks) * app->clocks_length);
	if (app->clocks == NULL) {
		LOG_ERROR("Failed to create clocks!\n");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < app->clocks_length; ++i) {
		app->clocks[i].fonts.time = NULL;
		app->clocks[i].fonts.mode = NULL;
		app->clocks[i].textures.current = NULL;
		app->clocks[i].textures.previous = NULL;
		app->clocks[i].waiting = false;
		app->clocks[i].running = true;
		SDL_Rect display_bounds;
		SDL_GetDisplayBounds(i, &display_bounds);
		if (app->properties.full)
			app->clocks[i].window = SDL_CreateWindow(
				PROGRAM_TITLE, display_bounds.x,
				display_bounds.y, display_bounds.w,
				display_bounds.h, flags);
		else
			app->clocks[i].window = SDL_CreateWindow(
				PROGRAM_TITLE,
				display_bounds.x +
					(display_bounds.w - WINDOW_WIDTH) / 2,
				display_bounds.y +
					(display_bounds.h - WINDOW_HEIGHT) / 2,
				WINDOW_WIDTH, WINDOW_HEIGHT, flags);
		if (app->clocks[i].window == NULL) {
			LOG_ERROR("%s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		// Init window size after create it.
		SDL_GetWindowSize(app->clocks[i].window, &app->clocks[i].width,
				  &app->clocks[i].height);
		app->clocks[i].renderer = SDL_CreateRenderer(
			app->clocks[i].window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE |
				SDL_RENDERER_PRESENTVSYNC);
		if (app->clocks[i].renderer == NULL) {
			LOG_ERROR("%s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		SDL_SetRenderDrawBlendMode(app->clocks[i].renderer,
					   SDL_BLENDMODE_BLEND);
	}
}

#ifdef _WIN32
void _flipclock_create_clocks_preview(struct flipclock *app)
{
	// Don't set fullscreen if in preview.
	app->properties.full = false;
	app->clocks = malloc(sizeof(*app->clocks) * app->clocks_length);
	if (app->clocks == NULL) {
		LOG_ERROR("Failed to create clocks!\n");
		exit(EXIT_FAILURE);
	}
	app->clocks[0].fonts.time = NULL;
	app->clocks[0].fonts.mode = NULL;
	app->clocks[0].textures.current = NULL;
	app->clocks[0].textures.previous = NULL;
	app->clocks[0].waiting = false;
	app->clocks[0].running = true;
	// Create window from native window when in preview.
	app->clocks[0].window =
		SDL_CreateWindowFrom(app->properties.preview_window);
	// Init window size after create it.
	SDL_GetWindowSize(app->clocks[0].window, &app->clocks[0].width,
			  &app->clocks[0].height);
	if (app->clocks[0].window == NULL) {
		LOG_ERROR("%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	app->clocks[0].renderer = SDL_CreateRenderer(
		app->clocks[0].window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE |
			SDL_RENDERER_PRESENTVSYNC);
	if (app->clocks[0].renderer == NULL) {
		LOG_ERROR("%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetRenderDrawBlendMode(app->clocks[0].renderer,
				   SDL_BLENDMODE_BLEND);
}

void _flipclock_create_clocks_win32(struct flipclock *app)
{
	if (!app->properties.screensaver)
		SDL_DisableScreenSaver();
	if (app->properties.preview)
		_flipclock_create_clocks_preview(app);
	else
		_flipclock_create_clocks_default(app);
}
#endif

void flipclock_create_clocks(struct flipclock *app)
{
#ifdef _WIN32
	_flipclock_create_clocks_win32(app);
#else
	// Android and Linux should share the same code here.
	SDL_DisableScreenSaver();
	_flipclock_create_clocks_default(app);
#endif
}

void _flipclock_set_fullscreen(struct flipclock *app, int clock_index,
			       bool full)
{
	app->properties.full = full;
	// Let's find which display the clock is inside.
	SDL_Rect display_bounds;
	int clock_x;
	int clock_y;
	SDL_GetWindowPosition(app->clocks[clock_index].window, &clock_x,
			      &clock_y);
	int clock_center_x = clock_x + app->clocks[clock_index].width / 2;
	int clock_center_y = clock_y + app->clocks[clock_index].height / 2;
	int display_number = SDL_GetNumVideoDisplays();
	// If a clock is out of all displays it will be re-placed into the last.
	for (int i = 0; i < display_number; ++i) {
		SDL_GetDisplayBounds(i, &display_bounds);
		if (clock_center_x >= display_bounds.x &&
		    clock_center_x < display_bounds.x + display_bounds.w &&
		    clock_center_y >= display_bounds.y &&
		    clock_center_y < display_bounds.y + display_bounds.h) {
			LOG_DEBUG("Clock `%d` is inside display `%d`.\n",
				  clock_index, i);
			break;
		}
	}
	if (full) {
		// Move clocks to their placed displays.
		SDL_SetWindowPosition(app->clocks[clock_index].window,
				      display_bounds.x, display_bounds.y);
		SDL_SetWindowFullscreen(app->clocks[clock_index].window,
					SDL_WINDOW_FULLSCREEN_DESKTOP);
		SDL_GetWindowSize(app->clocks[clock_index].window,
				  &app->clocks[clock_index].width,
				  &app->clocks[clock_index].height);
		SDL_ShowCursor(SDL_DISABLE);
		LOG_DEBUG("Set clock `%d` to fullscreen with size `%dx%d`.\n",
			  clock_index, app->clocks[clock_index].width,
			  app->clocks[clock_index].height);
	} else {
		SDL_SetWindowFullscreen(app->clocks[clock_index].window, 0);
		/**
		 * We need to restore window first, because if started in
		 * fullscreen mode, it will be maximized when turning off
		 * fullscreen mode and we cannot set window size. Looks like
		 * a strange bug.
		 */
		SDL_RestoreWindow(app->clocks[clock_index].window);
		SDL_SetWindowSize(app->clocks[clock_index].window, WINDOW_WIDTH,
				  WINDOW_HEIGHT);
		SDL_SetWindowPosition(
			app->clocks[clock_index].window,
			display_bounds.x +
				(display_bounds.w - WINDOW_WIDTH) / 2,
			display_bounds.y +
				(display_bounds.h - WINDOW_HEIGHT) / 2);
		SDL_ShowCursor(SDL_ENABLE);
		LOG_DEBUG("Set clock `%d` to windowed.\n", clock_index);
	}
}

void flipclock_refresh(struct flipclock *app, int clock_index)
{
	if (app->clocks[clock_index].width < app->clocks[clock_index].height) {
		// Some user do love portrait.
		app->clocks[clock_index].time_height =
			(app->clocks[clock_index].height * 0.4 >
					 app->clocks[clock_index].width * 0.8 ?
				 app->clocks[clock_index].width * 0.8 :
				 app->clocks[clock_index].height * 0.4) *
			app->properties.rect_scale;
		int space = app->clocks[clock_index].height * 0.06;
		app->clocks[clock_index].radius =
			app->clocks[clock_index].time_height / 10;

		app->clocks[clock_index].cards.hour.rect.y =
			(app->clocks[clock_index].height -
			 2 * app->clocks[clock_index].time_height - space) /
			2;
		app->clocks[clock_index].cards.hour.rect.x =
			(app->clocks[clock_index].width -
			 app->clocks[clock_index].time_height) /
			2;
		app->clocks[clock_index].cards.hour.rect.w =
			app->clocks[clock_index].time_height;
		app->clocks[clock_index].cards.hour.rect.h =
			app->clocks[clock_index].time_height;

		app->clocks[clock_index].cards.minute.rect.y =
			app->clocks[clock_index].cards.hour.rect.y +
			app->clocks[clock_index].cards.hour.rect.h + space;
		app->clocks[clock_index].cards.minute.rect.x =
			app->clocks[clock_index].cards.hour.rect.x;
		app->clocks[clock_index].cards.minute.rect.w =
			app->clocks[clock_index].time_height;
		app->clocks[clock_index].cards.minute.rect.h =
			app->clocks[clock_index].time_height;
	} else {
		// But others love landscape.
		app->clocks[clock_index].time_height =
			(app->clocks[clock_index].width * 0.4 >
					 app->clocks[clock_index].height * 0.8 ?
				 app->clocks[clock_index].height * 0.8 :
				 app->clocks[clock_index].width * 0.4) *
			app->properties.rect_scale;
		int space = app->clocks[clock_index].width * 0.06;
		app->clocks[clock_index].radius =
			app->clocks[clock_index].time_height / 10;

		app->clocks[clock_index].cards.hour.rect.x =
			(app->clocks[clock_index].width -
			 2 * app->clocks[clock_index].time_height - space) /
			2;
		app->clocks[clock_index].cards.hour.rect.y =
			(app->clocks[clock_index].height -
			 app->clocks[clock_index].time_height) /
			2;
		app->clocks[clock_index].cards.hour.rect.w =
			app->clocks[clock_index].time_height;
		app->clocks[clock_index].cards.hour.rect.h =
			app->clocks[clock_index].time_height;

		app->clocks[clock_index].cards.minute.rect.x =
			app->clocks[clock_index].cards.hour.rect.x +
			app->clocks[clock_index].cards.hour.rect.w + space;
		app->clocks[clock_index].cards.minute.rect.y =
			app->clocks[clock_index].cards.hour.rect.y;
		app->clocks[clock_index].cards.minute.rect.w =
			app->clocks[clock_index].time_height;
		app->clocks[clock_index].cards.minute.rect.h =
			app->clocks[clock_index].time_height;
	}
	app->clocks[clock_index].mode_height =
		app->clocks[clock_index].time_height / 10;
	/**
	 * Use -2 * MAX_PROGRESS as initial value to ensure a full render
	 * when mainloop starts.
	 */
	app->clocks[clock_index].cards.hour.start_tick = -2 * MAX_PROGRESS;
	app->clocks[clock_index].cards.minute.start_tick = -2 * MAX_PROGRESS;
}

void flipclock_create_textures(struct flipclock *app, int clock_index)
{
	// Two transparent backend texture swap for tribuffer.
	app->clocks[clock_index].textures.current = SDL_CreateTexture(
		app->clocks[clock_index].renderer, 0, SDL_TEXTUREACCESS_TARGET,
		app->clocks[clock_index].width,
		app->clocks[clock_index].height);
	if (app->clocks[clock_index].textures.current == NULL) {
		LOG_ERROR("%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetTextureBlendMode(app->clocks[clock_index].textures.current,
				SDL_BLENDMODE_BLEND);
	app->clocks[clock_index].textures.previous = SDL_CreateTexture(
		app->clocks[clock_index].renderer, 0, SDL_TEXTUREACCESS_TARGET,
		app->clocks[clock_index].width,
		app->clocks[clock_index].height);
	if (app->clocks[clock_index].textures.previous == NULL) {
		LOG_ERROR("%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetTextureBlendMode(app->clocks[clock_index].textures.previous,
				SDL_BLENDMODE_BLEND);
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
	if (strlen(app->properties.font_path) != 0) {
		LOG_DEBUG("Using font_path `%s`.\n", app->properties.font_path);
		app->clocks[clock_index].fonts.time =
			TTF_OpenFont(app->properties.font_path,
				     app->clocks[clock_index].time_height *
					     app->properties.font_scale);
		app->clocks[clock_index].fonts.mode =
			TTF_OpenFont(app->properties.font_path,
				     app->clocks[clock_index].mode_height *
					     app->properties.font_scale);
	} else {
#if defined(_WIN32)
		char font_path[MAX_BUFFER_LENGTH];
		snprintf(font_path, MAX_BUFFER_LENGTH, "%s\\flipclock.ttf",
			 app->properties.program_dir);
		font_path[MAX_BUFFER_LENGTH - 1] = '\0';
		if (strlen(font_path) == MAX_BUFFER_LENGTH - 1) {
			LOG_ERROR("font_path too long, may fail to load.\n");
		}
#elif defined(__ANDROID__)
		// Directly under `app/src/main/assets` for Android APP.
		char *font_path = "flipclock.ttf";
#elif defined(__linux__)
		char *font_path = INSTALL_PREFIX "/share/fonts/flipclock.ttf";
#endif
		LOG_DEBUG("Using font_path `%s`.\n", font_path);
		app->clocks[clock_index].fonts.time = TTF_OpenFont(
			font_path, app->clocks[clock_index].time_height *
					   app->properties.font_scale);
		app->clocks[clock_index].fonts.mode = TTF_OpenFont(
			font_path, app->clocks[clock_index].mode_height *
					   app->properties.font_scale);
	}
	if (app->clocks[clock_index].fonts.time == NULL ||
	    app->clocks[clock_index].fonts.mode == NULL) {
		LOG_ERROR("%s\n", TTF_GetError());
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

void _flipclock_clear_texture(struct flipclock *app, int clock_index,
			      SDL_Texture *target_texture,
			      SDL_Color background_color)
{
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, target_texture);
	SDL_SetRenderDrawColor(app->clocks[clock_index].renderer,
			       background_color.r, background_color.g,
			       background_color.b, background_color.a);
	SDL_RenderClear(app->clocks[clock_index].renderer);
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, NULL);
}

void _flipclock_render_rounded_box(struct flipclock *app, int clock_index,
				   SDL_Texture *target_texture,
				   SDL_Rect target_rect, int radius)
{
	if (radius <= 1) {
		SDL_SetRenderTarget(app->clocks[clock_index].renderer,
				    target_texture);
		SDL_SetRenderDrawColor(app->clocks[clock_index].renderer,
				       app->colors.rect.r, app->colors.rect.g,
				       app->colors.rect.b, app->colors.rect.a);
		SDL_RenderFillRect(app->clocks[clock_index].renderer,
				   &target_rect);
		SDL_SetRenderTarget(app->clocks[clock_index].renderer, NULL);
		return;
	}

	SDL_SetRenderTarget(app->clocks[clock_index].renderer, target_texture);
	SDL_SetRenderDrawColor(app->clocks[clock_index].renderer,
			       app->colors.rect.r, app->colors.rect.g,
			       app->colors.rect.b, app->colors.rect.a);
	if (2 * radius > target_rect.w)
		radius = target_rect.w / 2;
	if (2 * radius > target_rect.h)
		radius = target_rect.h / 2;
	int x = 0;
	int y = radius;
	int d = 3 - 2 * radius;
	while (x <= y) {
		SDL_RenderDrawLine(
			app->clocks[clock_index].renderer,
			target_rect.x + radius - x, target_rect.y + radius - y,
			target_rect.x + target_rect.w - radius + x - 1,
			target_rect.y + radius - y);
		SDL_RenderDrawLine(app->clocks[clock_index].renderer,
				   target_rect.x + radius - x,
				   target_rect.y + target_rect.h - radius + y,
				   target_rect.x + target_rect.w - radius + x -
					   1,
				   target_rect.y + target_rect.h - radius + y);
		SDL_RenderDrawLine(
			app->clocks[clock_index].renderer,
			target_rect.x + radius - y, target_rect.y + radius - x,
			target_rect.x + target_rect.w - radius + y - 1,
			target_rect.y + radius - x);
		SDL_RenderDrawLine(app->clocks[clock_index].renderer,
				   target_rect.x + radius - y,
				   target_rect.y + target_rect.h - radius + x,
				   target_rect.x + target_rect.w - radius + y -
					   1,
				   target_rect.y + target_rect.h - radius + x);
		if (d < 0) {
			d = d + 4 * x + 6;
		} else {
			d = d + 4 * (x - y) + 10;
			--y;
		}
		++x;
	}
	SDL_Rect temp_rect;
	temp_rect.x = target_rect.x;
	temp_rect.y = target_rect.y + radius;
	temp_rect.w = target_rect.w;
	temp_rect.h = target_rect.h - 2 * radius;
	SDL_RenderFillRect(app->clocks[clock_index].renderer, &temp_rect);
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, NULL);
}

void _flipclock_render_text(struct flipclock *app, int clock_index,
			    SDL_Texture *target_texture, SDL_Rect target_rect,
			    TTF_Font *font, char text[])
{
	int len = strlen(text);
	if (len > 2) {
		// We can handle text longer than 2 chars, though.
		LOG_ERROR("Text length must be less than 3!");
		exit(EXIT_FAILURE);
	}
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, target_texture);
	// We render text every minute, so we don't need cache.
	for (int i = 0; i < len; i++) {
		/**
		 * See https://www.libsdl.org/projects/SDL_ttf/docs/SDL_ttf_42.html#SEC42.
		 * Normally shaded is enough, however we have a rounded box,
		 * and many fonts' boxes are too big compared with their
		 * characters, they just cover the rounded corner.
		 * So I have to use blended mode, because solid mode does not
		 * have anti-alias.
		 */
		SDL_Surface *text_surface = TTF_RenderGlyph_Blended(
			font, text[i], app->colors.font);
		if (text_surface == NULL) {
			LOG_ERROR("%s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		SDL_Texture *text_texture = SDL_CreateTextureFromSurface(
			app->clocks[clock_index].renderer, text_surface);
		if (text_texture == NULL) {
			LOG_ERROR("%s\n", SDL_GetError());
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
		SDL_RenderCopy(app->clocks[clock_index].renderer, text_texture,
			       NULL, &text_rect);
		SDL_DestroyTexture(text_texture);
	}
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, NULL);
}

void _flipclock_render_divider(struct flipclock *app, int clock_index,
			       SDL_Texture *target_texture,
			       SDL_Rect target_rect)
{
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, target_texture);
	// Don't be transparent, or you will not see divider.
	SDL_SetRenderDrawColor(app->clocks[clock_index].renderer,
			       app->colors.back.r, app->colors.back.g,
			       app->colors.back.b, app->colors.back.a);
	SDL_RenderFillRect(app->clocks[clock_index].renderer, &target_rect);
	SDL_SetRenderTarget(app->clocks[clock_index].renderer, NULL);
}

void _flipclock_render_texture(struct flipclock *app, int clock_index)
{
	SDL_Texture *swap = app->clocks[clock_index].textures.current;
	app->clocks[clock_index].textures.current =
		app->clocks[clock_index].textures.previous;
	app->clocks[clock_index].textures.previous = swap;

	_flipclock_clear_texture(app, clock_index,
				 app->clocks[clock_index].textures.current,
				 app->colors.transparent);

	/**
	 * Don't draw one card after another!
	 * I don't know why, but it seems SDL2 under Windows has a bug.
	 * If I draw text for one card, then draw background for another card.
	 * The background will be black. I've tested a lot and my code was
	 * correct. They just works under Linux! So I don't know why,
	 * but if I draw all backgrounds, then texts, and dividers,
	 * that is, in layer sequence, it just works.
	 */

	// Background.
	_flipclock_render_rounded_box(app, clock_index,
				      app->clocks[clock_index].textures.current,
				      app->clocks[clock_index].cards.hour.rect,
				      app->clocks[clock_index].radius);

	_flipclock_render_rounded_box(
		app, clock_index, app->clocks[clock_index].textures.current,
		app->clocks[clock_index].cards.minute.rect,
		app->clocks[clock_index].radius);

	// Text.
	char text[3];
	if (app->properties.ampm) {
		/**
		 * Just draw AM/PM text on hour card.
		 * Don't use strftime() here,
		 * because font only have `A`, `P`, `M`.
		 */
		SDL_Rect mode_rect;
		// How do I get those numbers? Test.
		mode_rect.x = app->clocks[clock_index].cards.hour.rect.x +
			      app->clocks[clock_index].time_height / 50;
		mode_rect.y = app->clocks[clock_index].cards.hour.rect.y +
			      app->clocks[clock_index].time_height -
			      app->clocks[clock_index].mode_height -
			      app->clocks[clock_index].time_height / 35;
		mode_rect.w = app->clocks[clock_index].mode_height * 2;
		mode_rect.h = app->clocks[clock_index].mode_height;
		snprintf(text, sizeof(text), "%cM",
			 app->times.now.tm_hour / 12 ? 'P' : 'A');
		_flipclock_render_text(
			app, clock_index,
			app->clocks[clock_index].textures.current, mode_rect,
			app->clocks[clock_index].fonts.mode, text);
	}

	/**
	 * MSVC does not support `%l` (` 1` - `12`),
	 * so we have to use `%I` (`01` - `12`), and trim zero.
	 */
	strftime(text, sizeof(text), app->properties.ampm ? "%I" : "%H",
		 &app->times.now);
	// Trim zero when using 12-hour clock.
	if (app->properties.ampm && text[0] == '0') {
		text[0] = text[1];
		text[1] = text[2];
	}
	_flipclock_render_text(app, clock_index,
			       app->clocks[clock_index].textures.current,
			       app->clocks[clock_index].cards.hour.rect,
			       app->clocks[clock_index].fonts.time, text);

	strftime(text, sizeof(text), "%M", &app->times.now);
	_flipclock_render_text(app, clock_index,
			       app->clocks[clock_index].textures.current,
			       app->clocks[clock_index].cards.minute.rect,
			       app->clocks[clock_index].fonts.time, text);

	// And cut the card!
	SDL_Rect divider_rect;
	divider_rect.h = app->clocks[clock_index].cards.hour.rect.h / 100;
	divider_rect.w = app->clocks[clock_index].cards.hour.rect.w;
	divider_rect.x = app->clocks[clock_index].cards.hour.rect.x;
	divider_rect.y =
		app->clocks[clock_index].cards.hour.rect.y +
		(app->clocks[clock_index].cards.hour.rect.h - divider_rect.h) /
			2;
	_flipclock_render_divider(app, clock_index,
				  app->clocks[clock_index].textures.current,
				  divider_rect);

	divider_rect.h = app->clocks[clock_index].cards.minute.rect.h / 100;
	divider_rect.w = app->clocks[clock_index].cards.minute.rect.w;
	divider_rect.x = app->clocks[clock_index].cards.minute.rect.x;
	divider_rect.y = app->clocks[clock_index].cards.minute.rect.y +
			 (app->clocks[clock_index].cards.minute.rect.h -
			  divider_rect.h) /
				 2;
	_flipclock_render_divider(app, clock_index,
				  app->clocks[clock_index].textures.current,
				  divider_rect);
}

void _flipclock_flip_card(struct flipclock *app, int clock_index,
			  struct card card)
{
	int progress = SDL_GetTicks() - card.start_tick;
	if (progress >= MAX_PROGRESS) {
		// It finished flipping, so we don't draw flipping.
		SDL_RenderCopy(app->clocks[clock_index].renderer,
			       app->clocks[clock_index].textures.current,
			       &card.rect, &card.rect);
		return;
	}

	// Draw the upper current digit and render it.
	SDL_Rect half_source_rect;
	half_source_rect.x = card.rect.x;
	half_source_rect.y = card.rect.y;
	half_source_rect.w = card.rect.w;
	half_source_rect.h = card.rect.h / 2;
	SDL_Rect half_target_rect;
	half_target_rect.x = card.rect.x;
	half_target_rect.y = card.rect.y;
	half_target_rect.w = card.rect.w;
	half_target_rect.h = card.rect.h / 2;
	SDL_RenderCopy(app->clocks[clock_index].renderer,
		       app->clocks[clock_index].textures.current,
		       &half_source_rect, &half_target_rect);

	// Draw the lower previous digit and render it.
	half_source_rect.x = card.rect.x;
	half_source_rect.y = card.rect.y + card.rect.h / 2;
	half_source_rect.w = card.rect.w;
	half_source_rect.h = card.rect.h / 2;
	half_target_rect.x = card.rect.x;
	half_target_rect.y = card.rect.y + card.rect.h / 2;
	half_target_rect.w = card.rect.w;
	half_target_rect.h = card.rect.h / 2;
	SDL_RenderCopy(app->clocks[clock_index].renderer,
		       app->clocks[clock_index].textures.previous,
		       &half_source_rect, &half_target_rect);

	/**
	 * Draw the flip part.
	 * Upper half is previous and lower half is current.
	 * Just custom the destination Rect, zoom will be done automatically.
	 */
	bool upper_half = progress <= HALF_PROGRESS;
	double scale =
		upper_half ? 1.0 - (1.0 * progress) / HALF_PROGRESS :
			     ((1.0 * progress) - HALF_PROGRESS) / HALF_PROGRESS;
	half_source_rect.x = card.rect.x;
	half_source_rect.y = card.rect.y + (upper_half ? 0 : card.rect.h / 2);
	half_source_rect.w = card.rect.w;
	half_source_rect.h = card.rect.h / 2;
	half_target_rect.x = card.rect.x;
	half_target_rect.y =
		card.rect.y +
		(upper_half ? card.rect.h / 2 * (1 - scale) : card.rect.h / 2);
	half_target_rect.w = card.rect.w;
	half_target_rect.h = card.rect.h / 2 * scale;
	SDL_RenderCopy(app->clocks[clock_index].renderer,
		       upper_half ? app->clocks[clock_index].textures.previous :
				    app->clocks[clock_index].textures.current,
		       &half_source_rect, &half_target_rect);
}

void _flipclock_animate(struct flipclock *app, int clock_index)
{
	SDL_SetRenderDrawColor(app->clocks[clock_index].renderer,
			       app->colors.back.r, app->colors.back.g,
			       app->colors.back.b, app->colors.back.a);
	SDL_RenderClear(app->clocks[clock_index].renderer);
	_flipclock_flip_card(app, clock_index,
			     app->clocks[clock_index].cards.hour);
	_flipclock_flip_card(app, clock_index,
			     app->clocks[clock_index].cards.minute);
	SDL_RenderPresent(app->clocks[clock_index].renderer);
}

void _flipclock_handle_window_event(struct flipclock *app, SDL_Event event)
{
	int clock_index = -1;
	for (int i = 0; i < app->clocks_length; ++i) {
		if (event.window.windowID ==
			    SDL_GetWindowID(app->clocks[i].window) &&
		    app->clocks[i].running) {
			clock_index = i;
			break;
		}
	}
	if (clock_index == -1) {
		LOG_ERROR("There is no running window that event belongs!\n");
		return;
	}
	switch (event.window.event) {
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		/**
		 * Only re-render when size changed.
		 * Windows may send event when size
		 * not changed, and cause strange bugs.
		 */
		if (event.window.data1 != app->clocks[clock_index].width ||
		    event.window.data2 != app->clocks[clock_index].height) {
			app->clocks[clock_index].width = event.window.data1;
			app->clocks[clock_index].height = event.window.data2;
			LOG_DEBUG("New window size for "
				  "clock `%d` is `%dx%d`.\n",
				  clock_index, app->clocks[clock_index].width,
				  app->clocks[clock_index].height);
			flipclock_destroy_textures(app, clock_index);
			flipclock_close_fonts(app, clock_index);
			flipclock_refresh(app, clock_index);
			flipclock_open_fonts(app, clock_index);
			flipclock_create_textures(app, clock_index);
			_flipclock_render_texture(app, clock_index);
		}
		break;
	case SDL_WINDOWEVENT_MINIMIZED:
		app->clocks[clock_index].waiting = true;
		break;
	// `RESTORED` is emitted after `MINIMIZED`.
	case SDL_WINDOWEVENT_RESTORED:
		app->clocks[clock_index].waiting = false;
		/**
		 * Sometimes when a window is restored, its texture get lost.
		 * Typically happens when we have two fullscreen clocks in
		 * one display, and the lower one is switched to top, and we
		 * have to re-render its texture.
		 */
		_flipclock_render_texture(app, clock_index);
		break;
	case SDL_WINDOWEVENT_CLOSE:
		app->clocks[clock_index].running = false;
		// Don't forget to destroy window actually.
		SDL_DestroyRenderer(app->clocks[clock_index].renderer);
		SDL_DestroyWindow(app->clocks[clock_index].window);
		/**
		 * See https://wiki.libsdl.org/SDL_EventType#SDL_QUIT.
		 * It seems that SDL will send SDL_QUIT automatically
		 * when all windows are closed, so we don't need to exit
		 * manually here.
		 */
		LOG_DEBUG("Clock `%d` closed!\n", clock_index);
		break;
	default:
		break;
	}
}

void _flipclock_handle_keydown(struct flipclock *app, SDL_Event event)
{
	switch (event.key.keysym.sym) {
	case SDLK_ESCAPE:
	case SDLK_q:
	case SDLK_AC_BACK:
		app->running = false;
		break;
	case SDLK_t:
		app->properties.ampm = !app->properties.ampm;
		for (int i = 0; i < app->clocks_length; ++i) {
			if (!app->clocks[i].running)
				continue;
			_flipclock_render_texture(app, i);
		}
		break;
	case SDLK_f:
		app->properties.full = !app->properties.full;
		for (int i = 0; i < app->clocks_length; ++i) {
			if (!app->clocks[i].running)
				continue;
			/**
			 * Setting to windowed mode from fullscreen mode will
			 * emit a size changed event for window, but setting
			 * to fullscreen mode from windowed mode has no event,
			 * so we have to manually refresh clocks here. Strange.
			 */
			if (app->properties.full) {
				flipclock_destroy_textures(app, i);
				flipclock_close_fonts(app, i);
			}
			_flipclock_set_fullscreen(app, i, app->properties.full);
			if (app->properties.full) {
				flipclock_refresh(app, i);
				flipclock_open_fonts(app, i);
				flipclock_create_textures(app, i);
				_flipclock_render_texture(app, i);
			}
		}
		break;
	default:
		break;
	}
}

void _flipclock_handle_event(struct flipclock *app, SDL_Event event)
{
	switch (event.type) {
#ifdef _WIN32
	/**
	 * There is a silly design in Windows' screensaver
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
			app->running = false;
		break;
#endif
	case SDL_WINDOWEVENT:
		_flipclock_handle_window_event(app, event);
		break;
#ifdef _WIN32
	/**
	 * If under Windows, and not in preview window,
	 * and it was called as a screensaver,
	 * just exit when user press mouse button or move it,
	 * or interactive with touch screen.
	 */
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEMOTION:
	case SDL_MOUSEWHEEL:
	case SDL_FINGERDOWN:
		if (!app->properties.preview && app->properties.screensaver)
			app->running = false;
		break;
#endif
	/**
	 * For touch devices, the most used function is
	 * changing type, so we use double tap for it,
	 * instead of toggling fullscreen.
	 * Double tap (less then 300ms) changes type.
	 */
	case SDL_FINGERUP:
		if (event.tfinger.timestamp <
		    app->last_touch + DOUBLE_TAP_INTERVAL_MS) {
			app->properties.ampm = !app->properties.ampm;
			for (int i = 0; i < app->clocks_length; ++i) {
				if (!app->clocks[i].running)
					continue;
				_flipclock_render_texture(app, i);
			}
		}
		app->last_touch = event.tfinger.timestamp;
		break;
	case SDL_KEYDOWN:
#ifdef _WIN32
		/**
		 * If under Windows, and not in preview window,
		 * and it was called as a screensaver.
		 * just exit when user press any key.
		 * But if it was not called as a screensaver,
		 * it only handles some special keys.
		 * Also, we do nothing when in preview.
		 */
		if (!app->properties.preview && app->properties.screensaver)
			app->running = false;
		else if (!app->properties.preview)
			_flipclock_handle_keydown(app, event);
#else
		// It's simple under Linux and Android.
		_flipclock_handle_keydown(app, event);
#endif
		break;
	case SDL_QUIT:
		app->running = false;
		break;
	default:
		break;
	}
}

void flipclock_run_mainloop(struct flipclock *app)
{
	SDL_Event event;
	// Clear event queue before running.
	while (SDL_PollEvent(&event))
		;
	// First frame when app starts.
	for (int i = 0; i < app->clocks_length; ++i) {
		if (!app->clocks[i].running)
			continue;
		_flipclock_render_texture(app, i);
		_flipclock_animate(app, i);
	}
	while (app->running) {
#ifdef _WIN32
		// Exit when preview window closed.
		if (app->properties.preview &&
		    !IsWindow(app->properties.preview_window))
			app->running = false;
#endif
		if (SDL_WaitEventTimeout(&event, 1000 / FPS))
			_flipclock_handle_event(app, event);
		time_t raw_time = time(NULL);
		app->times.now = *localtime(&raw_time);
		if (app->times.now.tm_hour != app->times.past.tm_hour ||
		    app->times.now.tm_min != app->times.past.tm_min) {
			LOG_DEBUG("Time changed, rendering texture.\n");
			for (int i = 0; i < app->clocks_length; ++i) {
				if (!app->clocks[i].running)
					continue;
				_flipclock_render_texture(app, i);
			}
		}
		if (app->times.now.tm_hour != app->times.past.tm_hour) {
			for (int i = 0; i < app->clocks_length; ++i) {
				if (!app->clocks[i].running)
					continue;
				app->clocks[i].cards.hour.start_tick =
					SDL_GetTicks();
			}
		}
		if (app->times.now.tm_min != app->times.past.tm_min) {
			for (int i = 0; i < app->clocks_length; ++i) {
				if (!app->clocks[i].running)
					continue;
				app->clocks[i].cards.minute.start_tick =
					SDL_GetTicks();
			}
		}
		// Pause when minimized.
		for (int i = 0; i < app->clocks_length; ++i)
			if (!app->clocks[i].waiting && app->clocks[i].running)
				_flipclock_animate(app, i);
		// Sync time when animation ends.
		app->times.past = app->times.now;
	}
}

void flipclock_destroy_clocks(struct flipclock *app)
{
	for (int i = 0; i < app->clocks_length; ++i) {
		// Already destroyed.
		if (!app->clocks[i].running)
			continue;
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
	free(app);
}

void flipclock_print_help(struct flipclock *app, char program_name[])
{
	printf("A simple flip clock screensaver using SDL2.\n");
	printf("Version " PROJECT_VERSION ".\n");
	printf("Usage: %s [OPTION...] <value>\n", program_name);
	printf("Options:\n");
	printf("\t%ch\t\tDisplay help then exit.\n", OPT_START);
	printf("\t%cv\t\tDisplay version then exit.\n", OPT_START);
#ifdef _WIN32
	printf("\t%cs\t\t(Windows only) "
	       "Required for starting screensaver in Windows.\n",
	       OPT_START);
	printf("\t%cc\t\t(Windows only) Dummy configuration dialog.\n",
	       OPT_START);
	printf("\t%cp <HWND>\t(Windows only) Preview in given window.\n",
	       OPT_START);
#endif
	printf("\t%cw\t\tRun as a window instead of fullscreen.\n", OPT_START);
	printf("\t%ct <12|24>\tToggle 12-hour clock format (AM/PM) "
	       "or 24-hour clock format.\n",
	       OPT_START);
	printf("\t%cf <font>\tLoad custom font path.\n", OPT_START);
	printf("Press Esc or q to exit.\n");
	printf("Press f to toggle fullscreen.\n");
	printf("Press t to toggle 12/24-hour clock format.\n");
	printf("Using configuration file %s.\n", app->properties.conf_path);
}
