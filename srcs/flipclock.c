/**
 * Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.one/)
 */
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "getarg.h"
#include "flipclock.h"
#include "clock.h"
#include "card.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FPS 60
#define MAX_PROGRESS 300
#define HALF_PROGRESS (MAX_PROGRESS / 2)
#define DOUBLE_TAP_INTERVAL_MS 300

#if defined(_WIN32)
static void _flipclock_get_program_dir_win32(char program_dir[])
{
	RETURN_IF_FAIL(program_dir != NULL);

	/**
	 * See https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew.
	 * GetModuleFileName is a macro to
	 * GetModuleFileNameW and GetModuleFileNameA.
	 */
	GetModuleFileName(NULL, program_dir, MAX_BUFFER_LENGTH);
	program_dir[MAX_BUFFER_LENGTH - 1] = '\0';
	if (strlen(program_dir) == MAX_BUFFER_LENGTH - 1)
		LOG_ERROR("`program_dir` too long, may fail to load.\n");
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
	app->last_touch_time = 0;
	app->last_touch_finger = 0;
	app->running = true;
	app->text_color.r = 0xd0;
	app->text_color.g = 0xd0;
	app->text_color.b = 0xd0;
	app->text_color.a = 0xff;
	app->box_color.r = 0x20;
	app->box_color.g = 0x20;
	app->box_color.b = 0x20;
	app->box_color.a = 0xff;
	app->background_color.r = 0x00;
	app->background_color.g = 0x00;
	app->background_color.b = 0x00;
	app->background_color.a = 0xff;
	app->ampm = false;
	app->full = true;
	app->show_second = false;
	app->font_path[0] = '\0';
	app->conf_path[0] = '\0';
	app->text_scale = 1.0;
	app->card_scale = 1.0;
#if defined(_WIN32)
	app->preview = false;
	app->screensaver = false;
	app->program_dir[0] = '\0';
	_flipclock_get_program_dir_win32(app->program_dir);
	LOG_DEBUG("Using `program_dir` `%s`.\n", app->program_dir);
#endif
#if defined(_WIN32)
	snprintf(app->font_path, MAX_BUFFER_LENGTH, "%s\\flipclock.ttf",
		 app->program_dir);
#elif defined(__ANDROID__)
	// Directly under `app/src/main/assets` for Android APP.
	strncpy(app->font_path, "flipclock.ttf", MAX_BUFFER_LENGTH);
#elif defined(__linux__) && !defined(__ANDROID__)
	strncpy(app->font_path, PACKAGE_DATADIR "/fonts/flipclock.ttf",
		MAX_BUFFER_LENGTH);
#endif
	app->font_path[MAX_BUFFER_LENGTH - 1] = '\0';
	if (strlen(app->font_path) == MAX_BUFFER_LENGTH - 1)
		LOG_ERROR("`font_path` too long, may fail to load.\n");
	time_t raw_time = time(NULL);
	app->now = *localtime(&raw_time);
	return app;
}

static int _flipclock_parse_key_value(char line[], char **key, char **value)
{
	RETURN_VAL_IF_FAIL(line != NULL, -6);
	RETURN_VAL_IF_FAIL(key != NULL, -7);
	RETURN_VAL_IF_FAIL(value != NULL, -8);

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
	// Only handle line start, this makes it easier for colors.
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

static int _flipclock_parse_color(const char rgba[], SDL_Color *color)
{
	RETURN_VAL_IF_FAIL(rgba != NULL, -5);
	RETURN_VAL_IF_FAIL(color != NULL, -6);

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

#if defined(_WIN32)
static FILE *_flipclock_open_conf_win32(char conf_path[],
					const char program_dir[])
{
	RETURN_VAL_IF_FAIL(conf_path != NULL, NULL);
	RETURN_VAL_IF_FAIL(program_dir != NULL, NULL);

	snprintf(conf_path, MAX_BUFFER_LENGTH, "%s\\flipclock.conf",
		 program_dir);
	conf_path[MAX_BUFFER_LENGTH - 1] = '\0';
	if (strlen(conf_path) == MAX_BUFFER_LENGTH - 1)
		LOG_ERROR("`conf_path` too long, may fail to load.\n");
	return fopen(conf_path, "r");
}
#elif defined(__linux__) && !defined(__ANDROID__)
static FILE *_flipclock_open_conf_linux(char *conf_path)
{
	RETURN_VAL_IF_FAIL(conf_path != NULL, NULL);

	FILE *conf = NULL;

	// Be a good program.
	const char *conf_dir = getenv("XDG_CONFIG_HOME");
	if (conf_dir != NULL && strlen(conf_dir) != 0) {
		snprintf(conf_path, MAX_BUFFER_LENGTH, "%s/flipclock.conf",
			 conf_dir);
		conf_path[MAX_BUFFER_LENGTH - 1] = '\0';
		if (strlen(conf_path) == MAX_BUFFER_LENGTH - 1)
			LOG_ERROR("`conf_path` too long, may fail to load.\n");
		conf = fopen(conf_path, "r");
		if (conf != NULL)
			return conf;
	}

	// Linux users should not be homeless. But we may in sandbox.
	const char *home = getenv("HOME");
	if (home != NULL && strlen(home) != 0) {
		snprintf(conf_path, MAX_BUFFER_LENGTH,
			 "%s/.config/flipclock.conf", home);
		conf_path[MAX_BUFFER_LENGTH - 1] = '\0';
		if (strlen(conf_path) == MAX_BUFFER_LENGTH - 1)
			LOG_ERROR("`conf_path` too long, may fail to load.\n");
		conf = fopen(conf_path, "r");
		if (conf != NULL)
			return conf;
	}

	strncpy(conf_path, PACKAGE_SYSCONFDIR "/flipclock.conf",
		MAX_BUFFER_LENGTH);
	conf_path[MAX_BUFFER_LENGTH - 1] = '\0';
	if (strlen(conf_path) == MAX_BUFFER_LENGTH - 1)
		LOG_ERROR("`conf_path` too long, may fail to load.\n");
	return fopen(conf_path, "r");
}
#endif

static void _flipclock_apply_key_value(struct flipclock *app, const char key[],
				       const char value[])
{
	RETURN_IF_FAIL(app != NULL);
	RETURN_IF_FAIL(key != NULL);
	RETURN_IF_FAIL(value != NULL);

	/**
	 * It's better to use a temp variable here,
	 * so when parsing failed we still have the default color.
	 */
	SDL_Color parsed_color;
	if (!strcmp(key, "ampm")) {
		if (!strcmp(value, "true"))
			app->ampm = true;
	} else if (!strcmp(key, "full")) {
		if (!strcmp(value, "false"))
			app->full = false;
	} else if (!strcmp(key, "show_second")) {
		if (!strcmp(value, "true"))
			app->show_second = true;
	} else if (!strcmp(key, "font")) {
		strncpy(app->font_path, value, MAX_BUFFER_LENGTH);
		app->font_path[MAX_BUFFER_LENGTH - 1] = '\0';
		if (strlen(app->font_path) == MAX_BUFFER_LENGTH - 1)
			LOG_ERROR("`font_path` too long, may fail to load.\n");
	} else if (!strcmp(key, "text_scale")) {
		app->text_scale = strtod(value, NULL);
	} else if (!strcmp(key, "font_scale")) {
		// Backward compatibility for deprecated keys.
		LOG_ERROR("`font_scale` is deprecated, "
			  "use `text_scale` instead.\n");
		app->text_scale = strtod(value, NULL);
	} else if (!strcmp(key, "card_scale")) {
		app->card_scale = strtod(value, NULL);
	} else if (!strcmp(key, "rect_scale")) {
		// Backward compatibility for deprecated keys.
		LOG_ERROR("`rect_scale` is deprecated, "
			  "use `card_scale` instead.\n");
		app->card_scale = strtod(value, NULL);
	} else if (!strcmp(key, "text_color")) {
		if (!_flipclock_parse_color(value, &parsed_color))
			app->text_color = parsed_color;
		else
			LOG_ERROR("Failed to parse `text_color`!\n");
	} else if (!strcmp(key, "font_color")) {
		// Backward compatibility for deprecated keys.
		LOG_ERROR("`font_color` is deprecated, "
			  "use `text_color` instead.\n");
		if (!_flipclock_parse_color(value, &parsed_color))
			app->text_color = parsed_color;
		else
			LOG_ERROR("Failed to parse `text_color`!\n");
	} else if (!strcmp(key, "box_color")) {
		if (!_flipclock_parse_color(value, &parsed_color))
			app->box_color = parsed_color;
		else
			LOG_ERROR("Failed to parse `box_color`!\n");
	} else if (!strcmp(key, "rect_color")) {
		// Backward compatibility for deprecated keys.
		LOG_ERROR("`rect_color` is deprecated, "
			  "use `box_color` instead.\n");
		if (!_flipclock_parse_color(value, &parsed_color))
			app->box_color = parsed_color;
		else
			LOG_ERROR("Failed to parse `box_color`!\n");
	} else if (!strcmp(key, "background_color")) {
		if (!_flipclock_parse_color(value, &parsed_color))
			app->background_color = parsed_color;
		else
			LOG_ERROR("Failed to parse `background_color`!\n");
	} else if (!strcmp(key, "back_color")) {
		// Backward compatibility for deprecated keys.
		LOG_ERROR("`back_color` is deprecated, "
			  "use `background_color` instead.\n");
		if (!_flipclock_parse_color(value, &parsed_color))
			app->background_color = parsed_color;
		else
			LOG_ERROR("Failed to parse `background_color`!\n");
	} else {
		LOG_ERROR("Unknown key `%s`.\n", key);
	}
}

void flipclock_load_conf(struct flipclock *app)
{
	RETURN_IF_FAIL(app != NULL);

	FILE *conf = NULL;
#if defined(_WIN32)
	conf = _flipclock_open_conf_win32(app->conf_path, app->program_dir);
#elif defined(__linux__) && !defined(__ANDROID__)
	conf = _flipclock_open_conf_linux(app->conf_path);
#endif
	// Should never happen, but it's fine.
	if (conf == NULL)
		return;
#if !defined(__ANDROID__)
	LOG_DEBUG("Parsing `%s`.\n", app->conf_path);
#endif /**
	 * Most file systems have max file name length limit.
	 * So I don't need to alloc memory dynamically.
	 */
	char conf_line[MAX_BUFFER_LENGTH];
	char *key;
	char *value;
	while (fgets(conf_line, MAX_BUFFER_LENGTH, conf) != NULL) {
		if (strlen(conf_line) == MAX_BUFFER_LENGTH - 1)
			LOG_ERROR("`conf_line` too long, may fail to load.\n");
		if (_flipclock_parse_key_value(conf_line, &key, &value))
			continue;
		LOG_DEBUG("Parsed key `%s` and value `%s`.\n", key, value);
		_flipclock_apply_key_value(app, key, value);
	}
	fclose(conf);
}

static void _flipclock_create_clocks(struct flipclock *app)
{
	RETURN_IF_FAIL(app != NULL);

	// Create window for each display if fullscreen.
	if (app->full) {
		/**
		 * Instead of handling display number changing,
		 * let user restart program is easier.
		 */
		app->clocks_length = SDL_GetNumVideoDisplays();
		SDL_ShowCursor(SDL_DISABLE);
	}
	// I know what I am doing, silly tidy tools.
	// NOLINTNEXTLINE(bugprone-sizeof-expression)
	app->clocks = malloc(sizeof(*app->clocks) * app->clocks_length);
	if (app->clocks == NULL) {
		LOG_ERROR("Failed to create clocks!\n");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < app->clocks_length; ++i)
		app->clocks[i] = flipclock_clock_create(app, i);
}

#if defined(_WIN32)
/**
 * There is another silly design in Windows screensaver chooser (yes, differs
 * from the one happens when you choose other screensaver and then choose
 * back!). When you press preview button to start a fullscreen screensaver and
 * close it, Windows will launch another process in the small preview window!
 * And even SDL_RENDER_TARGETS_RESET is not sent this time! So there is no
 * other way than creating lock files by ourselves, what a horrible system!
 */

// Have to use global variable here because of atexit().
char preview_lock_path[MAX_BUFFER_LENGTH] = { '\0' };

static void _flipclock_get_preview_lock_path_win32(HWND preview_window,
						   const char program_dir[])
{
	RETURN_IF_FAIL(program_dir != NULL);

	/**
	 * User can open more than screensaver chooser,
	 * so we need to add HWND as part of lock file name.
	 */
	snprintf(preview_lock_path, MAX_BUFFER_LENGTH, "%s\\flipclock.%lu.lock",
		 program_dir, preview_window);
	preview_lock_path[MAX_BUFFER_LENGTH - 1] = '\0';
	if (strlen(preview_lock_path) == MAX_BUFFER_LENGTH - 1)
		LOG_ERROR("`preview_lock_path` too long, may fail to load.\n");
}

static void _flipclock_remove_preview_lock_win32(void)
{
	/**
	 * UNIX is designed for normal people, because you can remove one
	 * file after you just opened it, system will close it until your
	 * program ends. While silly Windows just say "cannot modify an opened
	 * file". So this function is used for atexit(), we have to remove
	 * file by ourselves at exit.
	 */
	remove(preview_lock_path);
}

static void _flipclock_lock_preview_win32(struct flipclock *app)
{
	RETURN_IF_FAIL(app != NULL);

	_flipclock_get_preview_lock_path_win32(app->preview_window,
					       app->program_dir);
	LOG_DEBUG("Using `preview_lock_path` `%s`.\n", preview_lock_path);
	FILE *preview_lock = fopen(preview_lock_path, "r");
	if (preview_lock != NULL || errno != ENOENT) {
		LOG_ERROR("Already running in the given preview window!\n");
		fclose(preview_lock);
		exit(EXIT_FAILURE);
	}
	preview_lock = fopen(preview_lock_path, "w");
	fprintf(preview_lock, "%lu\n", app->preview_window);
	fclose(preview_lock);
	atexit(_flipclock_remove_preview_lock_win32);
}

static void _flipclock_create_preview_win32(struct flipclock *app)
{
	RETURN_IF_FAIL(app != NULL);

	_flipclock_lock_preview_win32(app);
	app->clocks = malloc(sizeof(*app->clocks) * app->clocks_length);
	if (app->clocks == NULL) {
		LOG_ERROR("Failed to create clocks!\n");
		exit(EXIT_FAILURE);
	}
	// Create window from native window when in preview.
	app->clocks[0] = flipclock_clock_create_preview(app);
}

static void _flipclock_create_clocks_win32(struct flipclock *app)
{
	RETURN_IF_FAIL(app != NULL);

	if (!app->screensaver)
		SDL_DisableScreenSaver();
	if (app->preview)
		_flipclock_create_preview_win32(app);
	else
		_flipclock_create_clocks(app);
}
#endif

void flipclock_create_clocks(struct flipclock *app)
{
	RETURN_IF_FAIL(app != NULL);

#if defined(_WIN32)
	_flipclock_create_clocks_win32(app);
#else
	// Android and Linux should share the same code here.
	SDL_DisableScreenSaver();
	_flipclock_create_clocks(app);
#endif
}

static void _flipclock_set_show_second(struct flipclock *app, bool show_second)
{
	RETURN_IF_FAIL(app != NULL);

	app->show_second = show_second;
	for (int i = 0; i < app->clocks_length; ++i) {
		if (app->clocks[i] == NULL)
			continue;
		flipclock_clock_set_show_second(app->clocks[i], show_second);
	}
}

static void _flipclock_set_fullscreen(struct flipclock *app, bool full)
{
	RETURN_IF_FAIL(app != NULL);

	app->full = full;
	if (full)
		SDL_ShowCursor(SDL_DISABLE);
	else
		SDL_ShowCursor(SDL_ENABLE);
	for (int i = 0; i < app->clocks_length; ++i) {
		if (app->clocks[i] == NULL)
			continue;
		flipclock_clock_set_fullscreen(app->clocks[i], full);
	}
}

/**
 * If you changed `ampm`, you must call `_flipclock_set_hour()` after it,
 * because hour number will change in differet types.
 */
static void _flipclock_set_ampm(struct flipclock *app, bool ampm)
{
	RETURN_IF_FAIL(app != NULL);

	app->ampm = ampm;
	if (app->ampm) {
		for (int i = 0; i < app->clocks_length; ++i) {
			if (app->clocks[i] == NULL)
				continue;
			char text[3];
			snprintf(text, sizeof(text), "%cM",
				 app->now.tm_hour / 12 ? 'P' : 'A');
			flipclock_clock_set_ampm(app->clocks[i], text);
		}
	} else {
		for (int i = 0; i < app->clocks_length; ++i) {
			if (app->clocks[i] == NULL)
				continue;
			flipclock_clock_set_ampm(app->clocks[i], NULL);
		}
	}
}

static void _flipclock_set_hour(struct flipclock *app, bool flip)
{
	RETURN_IF_FAIL(app != NULL);

	for (int i = 0; i < app->clocks_length; ++i) {
		if (app->clocks[i] == NULL)
			continue;
		char text[3];
		strftime(text, sizeof(text), app->ampm ? "%I" : "%H",
			 &app->now);
		// Trim zero when using 12-hour clock.
		if (app->ampm && text[0] == '0') {
			text[0] = text[1];
			text[1] = text[2];
		}
		flipclock_clock_set_hour(app->clocks[i], text, flip);
	}
}

static void _flipclock_set_minute(struct flipclock *app, bool flip)
{
	RETURN_IF_FAIL(app != NULL);

	for (int i = 0; i < app->clocks_length; ++i) {
		if (app->clocks[i] == NULL)
			continue;
		char text[3];
		strftime(text, sizeof(text), "%M", &app->now);
		flipclock_clock_set_minute(app->clocks[i], text, flip);
	}
}

static void _flipclock_set_second(struct flipclock *app, bool flip)
{
	RETURN_IF_FAIL(app != NULL);

	for (int i = 0; i < app->clocks_length; ++i) {
		if (app->clocks[i] == NULL)
			continue;
		char text[3];
		strftime(text, sizeof(text), "%S", &app->now);
		flipclock_clock_set_second(app->clocks[i], text, flip);
	}
}

static void _flipclock_animate(struct flipclock *app)
{
	RETURN_IF_FAIL(app != NULL);

	// Pause when minimized.
	for (int i = 0; i < app->clocks_length; ++i) {
		if (app->clocks[i] == NULL)
			continue;
		if (!app->clocks[i]->waiting)
			flipclock_clock_animate(app->clocks[i]);
	}
}

static void _flipclock_handle_window_event(struct flipclock *app,
					   SDL_Event event)
{
	RETURN_IF_FAIL(app != NULL);

	struct flipclock_clock *clock = NULL;
	for (int i = 0; i < app->clocks_length; ++i) {
		// Ignore closed clocks.
		if (app->clocks[i] == NULL)
			continue;
		if (event.window.windowID ==
		    SDL_GetWindowID(app->clocks[i]->window)) {
			clock = app->clocks[i];
			break;
		}
	}
	if (clock == NULL) {
		LOG_ERROR("There is no running window that event belongs!\n");
		// It should be safe to ignore this event.
		return;
	}
	flipclock_clock_handle_window_event(clock, event);
}

static void _flipclock_handle_keydown(struct flipclock *app, SDL_Event event)
{
	RETURN_IF_FAIL(app != NULL);

	switch (event.key.keysym.sym) {
	case SDLK_ESCAPE:
	case SDLK_q:
	case SDLK_AC_BACK:
		app->running = false;
		break;
	case SDLK_t:
		LOG_DEBUG("Key `t` pressed.\n");
		_flipclock_set_ampm(app, !app->ampm);
		_flipclock_set_hour(app, false);
		break;
	case SDLK_f:
		LOG_DEBUG("Key `f` pressed.\n");
		_flipclock_set_fullscreen(app, !app->full);
		break;
	case SDLK_s:
		LOG_DEBUG("Key `s` pressed.\n");
		_flipclock_set_show_second(app, !app->show_second);
		// Must set second text, because created card has empty text.
		_flipclock_set_second(app, false);
		break;
	default:
		break;
	}
}

static void _flipclock_handle_event(struct flipclock *app, SDL_Event event)
{
	RETURN_IF_FAIL(app != NULL);

	switch (event.type) {
#if defined(_WIN32)
	/**
	 * There is a silly design in Windows screensaver
	 * chooser. When you choose one screensaver, it will
	 * run the program with `/p HWND`, but if you changed
	 * to another, the former will not receive close
	 * event (yes, any kind of close event is not sent),
	 * and if you choose the former again, it will run
	 * the program with the same HWND again! And your
	 * previous program only get a SDL_RENDER_TARGETS_RESET.
	 * So we have to close the lost program manually here.
	 */
	case SDL_RENDER_TARGETS_RESET:
		if (app->preview)
			app->running = false;
		break;
#endif
	case SDL_WINDOWEVENT:
		_flipclock_handle_window_event(app, event);
		break;
#if defined(_WIN32)
	/**
	 * If under Windows, and not in preview window,
	 * and it was called as a screensaver,
	 * just exit when user press mouse button or move it,
	 * or interactive with touch screen.
	 */
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEMOTION:
	case SDL_MOUSEWHEEL:
		if (!app->preview && app->screensaver)
			app->running = false;
		break;
#endif
	case SDL_FINGERDOWN:
#if defined(_WIN32)
		if (!app->preview && app->screensaver)
			app->running = false;
		break;
#else
		// TODO: May not work, 3 fingers contains 2 fingers.
		switch (SDL_GetNumTouchFingers(event.tfinger.touchId)) {
		case 2:
			LOG_DEBUG("2 finger touch detected!\n");
			_flipclock_set_ampm(app, !app->ampm);
			// Setting ampm always changes hour.
			_flipclock_set_hour(app, false);
			break;
		case 3:
			LOG_DEBUG("3 finger touch detected!\n");
			_flipclock_set_show_second(app, !app->show_second);
			/**
			 * Must set second text, because created card has empty
			 * text.
			 */
			_flipclock_set_second(app, false);
			break;
		default:
			break;
		}
		break;
#endif

	/**
	 * For touch devices, the most used function is
	 * changing type, so we use double tap for it,
	 * instead of toggling fullscreen.
	 * Double tap (less then 300ms) changes type.
	 */
	case SDL_FINGERUP:
		if (event.tfinger.fingerId == app->last_touch_finger &&
		    event.tfinger.timestamp <
			    app->last_touch_time + DOUBLE_TAP_INTERVAL_MS) {
			LOG_DEBUG("Double tap detected.\n");
			_flipclock_set_ampm(app, !app->ampm);
			_flipclock_set_hour(app, false);
		}
		app->last_touch_time = event.tfinger.timestamp;
		app->last_touch_finger = event.tfinger.fingerId;
		break;
	case SDL_KEYDOWN:
#if defined(_WIN32)
		/**
		 * If under Windows, and not in preview window,
		 * and it was called as a screensaver.
		 * just exit when user press any key.
		 * But if it was not called as a screensaver,
		 * it only handles some special keys.
		 * Also, we do nothing when in preview.
		 */
		if (!app->preview && app->screensaver)
			app->running = false;
		else if (!app->preview)
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
	RETURN_IF_FAIL(app != NULL);

	SDL_Event event;
	// Clear event queue before running.
	while (SDL_PollEvent(&event))
		;
	// First frame when app starts.
	_flipclock_set_ampm(app, app->ampm);
	_flipclock_set_hour(app, false);
	_flipclock_set_minute(app, false);
	if (app->show_second)
		_flipclock_set_second(app, false);
	_flipclock_animate(app);
	while (app->running) {
#if defined(_WIN32)
		// Exit when preview window closed.
		if (app->preview && !IsWindow(app->preview_window))
			app->running = false;
#endif
		if (SDL_WaitEventTimeout(&event, 1000 / FPS))
			_flipclock_handle_event(app, event);
		struct tm past = app->now;
		time_t raw_time = time(NULL);
		app->now = *localtime(&raw_time);
		if (app->now.tm_hour != past.tm_hour) {
			_flipclock_set_ampm(app, app->ampm);
			_flipclock_set_hour(app, true);
		}
		if (app->now.tm_min != past.tm_min)
			_flipclock_set_minute(app, true);
		if (app->show_second && app->now.tm_sec != past.tm_sec)
			_flipclock_set_second(app, true);
		_flipclock_animate(app);
	}
}

void flipclock_destroy_clocks(struct flipclock *app)
{
	RETURN_IF_FAIL(app != NULL);

	for (int i = 0; i < app->clocks_length; ++i) {
		if (app->clocks[i] == NULL)
			continue;
		flipclock_clock_destroy(app->clocks[i]);
	}
	free(app->clocks);
	if (app->full)
		SDL_ShowCursor(SDL_ENABLE);
#if defined(_WIN32)
	if (!app->screensaver)
		SDL_EnableScreenSaver();
#else
	SDL_EnableScreenSaver();
#endif
}

void flipclock_destroy(struct flipclock *app)
{
	RETURN_IF_FAIL(app != NULL);

	free(app);
}

void flipclock_print_help(struct flipclock *app, char program_name[])
{
	RETURN_IF_FAIL(app != NULL);
	RETURN_IF_FAIL(program_name != NULL);

	printf("A simple flip clock screensaver using SDL2.\n");
#if !defined(__ANDROID__)
	printf("Version " PROJECT_VERSION ".\n");
#endif
	printf("Usage: %s [OPTION...] <value>\n", program_name);
	printf("Options:\n");
	printf("\t%ch\t\tDisplay help then exit.\n", OPT_START);
	printf("\t%cv\t\tDisplay version then exit.\n", OPT_START);
#if defined(_WIN32)
	printf("\t%cs\t\t(Windows only) "
	       "Required for starting screensaver in Windows.\n",
	       OPT_START);
	printf("\t%cc\t\t(Windows only) Dummy configuration dialog.\n",
	       OPT_START);
	printf("\t%cp <HWND>\t(Windows only) Preview in given window.\n",
	       OPT_START);
#endif
	printf("\t%c3\t\tShow second.\n", OPT_START);
	printf("\t%cw\t\tRun as a window instead of fullscreen.\n", OPT_START);
	printf("\t%ct <12|24>\tToggle 12-hour clock format (AM/PM) "
	       "or 24-hour clock format.\n",
	       OPT_START);
	printf("\t%cf <font>\tLoad custom font path.\n", OPT_START);
	printf("Press Esc or q to exit.\n");
	printf("Press s to toggle second.\n");
	printf("Press f to toggle fullscreen.\n");
	printf("Press t to toggle 12/24-hour clock format.\n");
	printf("Using configuration file %s.\n", app->conf_path);
}
