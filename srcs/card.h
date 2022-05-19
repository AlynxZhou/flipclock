#ifndef __CARD_H__
#define __CARD_H__

#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

// I am not creating a textarea.
#define MAX_TEXT_LENGTH 8

struct flipclock_card {
	struct flipclock *app;
	SDL_Renderer *renderer;
	SDL_Texture *current;
	SDL_Texture *previous;
	bool should_redraw;
	long long start_tick;
	SDL_Rect rect;
	char text[MAX_TEXT_LENGTH];
	TTF_Font *font;
	bool has_sub_text;
	SDL_Rect sub_rect;
	char sub_text[MAX_TEXT_LENGTH];
	TTF_Font *sub_font;
	int divider_height;
	int radius;
};

struct flipclock_card *flipclock_card_create(struct flipclock *app,
					     SDL_Renderer *renderer);
void flipclock_card_set_rect(struct flipclock_card *card, const SDL_Rect rect);
void flipclock_card_set_text(struct flipclock_card *card, const char text[]);
void flipclock_card_set_sub_text(struct flipclock_card *card,
				 const char sub_text[]);
void flipclock_card_flip(struct flipclock_card *card);
void flipclock_card_animate(struct flipclock_card *card);
void flipclock_card_destory(struct flipclock_card *card);

#endif
