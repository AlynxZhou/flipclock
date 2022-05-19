#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "flipclock.h"
#include "clock.h"
#include "card.h"

#define PI 3.1415927
#define MAX_PROGRESS 300
#define HALF_PROGRESS (MAX_PROGRESS / 2)

struct flipclock_card *flipclock_card_create(struct flipclock *app,
					     SDL_Renderer *renderer)
{
	struct flipclock_card *card = malloc(sizeof(*card));
	if (card == NULL) {
		LOG_ERROR("Failed to create card!");
		exit(EXIT_FAILURE);
	}
	card->app = app;
	card->renderer = renderer;
	card->current = NULL;
	card->previous = NULL;
	card->should_redraw = false;
	card->start_tick = 0;
	card->text[0] = '\0';
	card->font = NULL;
	card->has_sub_text = false;
	card->sub_text[0] = '\0';
	card->sub_font = NULL;
	card->divider_height = 0;
	card->rect.w = 0;
	card->rect.h = 0;
	return card;
}

static void _flipclock_card_create_textures(struct flipclock_card *card)
{
	LOG_DEBUG("Creating new textures with size %dx%d.\n", card->rect.w,
		  card->rect.h);
	card->current = SDL_CreateTexture(card->renderer, 0,
					  SDL_TEXTUREACCESS_TARGET,
					  card->rect.w, card->rect.h);
	if (card->current == NULL) {
		LOG_ERROR("%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetTextureBlendMode(card->current, SDL_BLENDMODE_BLEND);
	card->previous = SDL_CreateTexture(card->renderer, 0,
					   SDL_TEXTUREACCESS_TARGET,
					   card->rect.w, card->rect.h);
	if (card->previous == NULL) {
		LOG_ERROR("%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetTextureBlendMode(card->previous, SDL_BLENDMODE_BLEND);
}

static void _flipclock_card_destroy_textures(struct flipclock_card *card)
{
	LOG_DEBUG("Destroying old textures.\n");
	if (card->current != NULL) {
		SDL_DestroyTexture(card->current);
		card->current = NULL;
	}
	if (card->previous != NULL) {
		SDL_DestroyTexture(card->previous);
		card->previous = NULL;
	}
}

// TODO: Only open sub font if sub text used.
static void _flipclock_card_open_fonts(struct flipclock_card *card)
{
	const struct flipclock *app = card->app;
	LOG_DEBUG("Opening font from `%s`.\n", app->font_path);
	card->font =
		TTF_OpenFont(app->font_path, card->rect.h * app->text_scale);
	card->sub_font = TTF_OpenFont(app->font_path,
				      card->sub_rect.h * app->text_scale);
	if (card->font == NULL || card->sub_font == NULL) {
		LOG_ERROR("%s\n", TTF_GetError());
		exit(EXIT_FAILURE);
	}
}

static void _flipclock_card_close_fonts(struct flipclock_card *card)
{
	LOG_DEBUG("Closing old font.\n");
	if (card->font != NULL) {
		TTF_CloseFont(card->font);
		card->font = NULL;
	}
	if (card->sub_font != NULL) {
		TTF_CloseFont(card->sub_font);
		card->sub_font = NULL;
	}
}

static void _flipclock_card_clear_current_texture(struct flipclock_card *card)
{
	SDL_SetRenderTarget(card->renderer, card->current);
	// Always clear texture with transparent so rounded corner will be fine.
	SDL_SetRenderDrawColor(card->renderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(card->renderer);
	SDL_SetRenderTarget(card->renderer, NULL);
}

static void _flipclock_card_draw_rounded_box(struct flipclock_card *card)
{
	const struct flipclock *app = card->app;
	// Card-local position.
	const SDL_Rect box_rect = { 0, 0, card->rect.w, card->rect.h };
	if (2 * card->radius > box_rect.w)
		card->radius = box_rect.w / 2;
	if (2 * card->radius > box_rect.h)
		card->radius = box_rect.h / 2;
	// Worst case: a normal rect.
	if (card->radius <= 1) {
		SDL_SetRenderTarget(card->renderer, card->current);
		SDL_SetRenderDrawColor(card->renderer, app->box_color.r,
				       app->box_color.g, app->box_color.b,
				       app->box_color.a);
		SDL_RenderFillRect(card->renderer, &box_rect);
		SDL_SetRenderTarget(card->renderer, NULL);
		return;
	}

	SDL_SetRenderTarget(card->renderer, card->current);
	SDL_SetRenderDrawColor(card->renderer, app->box_color.r,
			       app->box_color.g, app->box_color.b,
			       app->box_color.a);
	int x = 0;
	int y = card->radius;
	int d = 3 - 2 * card->radius;
	while (x <= y) {
		SDL_RenderDrawLine(
			card->renderer, box_rect.x + card->radius - x,
			box_rect.y + card->radius - y,
			box_rect.x + box_rect.w - card->radius + x - 1,
			box_rect.y + card->radius - y);
		SDL_RenderDrawLine(
			card->renderer, box_rect.x + card->radius - x,
			box_rect.y + box_rect.h - card->radius + y,
			box_rect.x + box_rect.w - card->radius + x - 1,
			box_rect.y + box_rect.h - card->radius + y);
		SDL_RenderDrawLine(
			card->renderer, box_rect.x + card->radius - y,
			box_rect.y + card->radius - x,
			box_rect.x + box_rect.w - card->radius + y - 1,
			box_rect.y + card->radius - x);
		SDL_RenderDrawLine(
			card->renderer, box_rect.x + card->radius - y,
			box_rect.y + box_rect.h - card->radius + x,
			box_rect.x + box_rect.w - card->radius + y - 1,
			box_rect.y + box_rect.h - card->radius + x);
		if (d < 0) {
			d = d + 4 * x + 6;
		} else {
			d = d + 4 * (x - y) + 10;
			--y;
		}
		++x;
	}
	SDL_Rect temp_rect;
	temp_rect.x = box_rect.x;
	temp_rect.y = box_rect.y + card->radius;
	temp_rect.w = box_rect.w;
	temp_rect.h = box_rect.h - 2 * card->radius;
	SDL_RenderFillRect(card->renderer, &temp_rect);
	SDL_SetRenderTarget(card->renderer, NULL);
}

// A special text drawing function, will draw all chars as mono.
static void _draw_text(SDL_Renderer *renderer, SDL_Texture *target_texture,
		       SDL_Rect target_rect, TTF_Font *font, SDL_Color color,
		       const char text[])
{
	int len = strlen(text);
	LOG_DEBUG("Drawing text `%s`.\n", text);
	SDL_SetRenderTarget(renderer, target_texture);
	for (int i = 0; i < len; ++i) {
		/**
		 * See https://www.libsdl.org/projects/SDL_ttf/docs/SDL_ttf_42.html#SEC42.
		 * Normally shaded is enough, however we have a rounded box,
		 * and many fonts' boxes are too big compared with their
		 * characters, they just cover the rounded corner.
		 * So I have to use blended mode, because solid mode does not
		 * have anti-alias.
		 */
		SDL_Surface *text_surface =
			TTF_RenderGlyph_Blended(font, text[i], color);
		if (text_surface == NULL) {
			LOG_ERROR("%s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		SDL_Texture *text_texture =
			SDL_CreateTextureFromSurface(renderer, text_surface);
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
		SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
		SDL_DestroyTexture(text_texture);
	}
	SDL_SetRenderTarget(renderer, NULL);
}

static void _flipclock_card_draw_text(struct flipclock_card *card)
{
	const struct flipclock *app = card->app;
	// Card-local position.
	const SDL_Rect box_rect = { 0, 0, card->rect.w, card->rect.h };
	_draw_text(card->renderer, card->current, box_rect, card->font,
		   app->text_color, card->text);
	if (card->has_sub_text) {
		_draw_text(card->renderer, card->current, card->sub_rect,
			   card->sub_font, app->text_color, card->sub_text);
	}
}

static void _flipclock_card_draw_divider(struct flipclock_card *card)
{
	const struct flipclock *app = card->app;
	SDL_Rect divider_rect = { 0, (card->rect.h - card->divider_height) / 2,
				  card->rect.w, card->divider_height };
	SDL_SetRenderTarget(card->renderer, card->current);
	// Don't be transparent, or you will not see divider, it's over card.
	SDL_SetRenderDrawColor(card->renderer, app->background_color.r,
			       app->background_color.g, app->background_color.b,
			       app->background_color.a);
	SDL_RenderFillRect(card->renderer, &divider_rect);
	SDL_SetRenderTarget(card->renderer, NULL);
}

static void _flipclock_card_draw(struct flipclock_card *card)
{
	// Always do texture swap before drawing.
	SDL_Texture *swap = card->current;
	card->current = card->previous;
	card->previous = swap;

	_flipclock_card_clear_current_texture(card);

	LOG_DEBUG("Drawing card.\n");
	_flipclock_card_draw_rounded_box(card);
	_flipclock_card_draw_text(card);
	_flipclock_card_draw_divider(card);
}

// Those setter functions will request redraw.
void flipclock_card_set_rect(struct flipclock_card *card, const SDL_Rect rect)
{
	card->divider_height = rect.h / 100;
	card->radius = rect.h / 10;
	card->sub_rect.h = rect.h / 10;
	// Sub text's width is decide by the height.
	card->sub_rect.w = card->sub_rect.h * strlen(card->sub_text);
	// This should be a card-local position, so don't add rect's x and y.
	card->sub_rect.x = rect.h / 50;
	card->sub_rect.y = rect.h - rect.h / 35 - card->sub_rect.h;
	// Reload textures and fonts if size changed.
	SDL_Rect old_rect = card->rect;
	card->rect = rect;
	if (card->rect.w != old_rect.w || card->rect.h != old_rect.h) {
		_flipclock_card_close_fonts(card);
		_flipclock_card_open_fonts(card);
		_flipclock_card_destroy_textures(card);
		_flipclock_card_create_textures(card);
	}
	// A redraw is requested because size or position changed.
	if (card->rect.x != old_rect.x || card->rect.y != old_rect.y ||
	    card->rect.w != old_rect.w || card->rect.h != old_rect.h)
		card->should_redraw = true;
}

void flipclock_card_set_text(struct flipclock_card *card, const char text[])
{
	// card->text_changed = true;
	if (text == NULL) {
		card->text[0] = '\0';
	} else {
		strncpy(card->text, text, MAX_TEXT_LENGTH);
		card->text[MAX_TEXT_LENGTH - 1] = '\0';
	}

	// TODO: Copy and compare text internally.
	// So we won't update card when we have many cards displaying a word
	// and change word.

	/**
	 * Setting text always requests a redraw, but not always requests a
	 * flipping. You don't want to flip when you change ampm.
	 */
	card->should_redraw = true;
}

void flipclock_card_set_sub_text(struct flipclock_card *card,
				 const char sub_text[])
{
	if (sub_text == NULL) {
		card->has_sub_text = false;
		card->sub_text[0] = '\0';
	} else {
		card->has_sub_text = true;
		strncpy(card->sub_text, sub_text, MAX_TEXT_LENGTH);
		card->sub_text[MAX_TEXT_LENGTH - 1] = '\0';
	}
	// Sub text length might be changed so re-calculate it.
	card->sub_rect.w = card->sub_rect.h * strlen(card->sub_text);

	/**
	 * Setting text always requests a redraw, but not always requests a
	 * flipping. You don't want to flip when you change ampm.
	 */
	card->should_redraw = true;
}

void flipclock_card_flip(struct flipclock_card *card)
{
	// Flipping animation start.
	card->start_tick = SDL_GetTicks();
}

void flipclock_card_animate(struct flipclock_card *card)
{
	/**
	 * We defer redraw requests to actually copy, so we only redraw card
	 * once for different text changes.
	 */
	if (card->should_redraw) {
		_flipclock_card_draw(card);
		card->should_redraw = false;
	}

	// Do the flipping animation by copy card to window's given position.

	long long progress = SDL_GetTicks() - card->start_tick;
	// Don't animate when program just started.
	if (progress >= MAX_PROGRESS || card->start_tick == 0) {
		// It finished flipping, so we don't draw flipping animation.
		// Card-local position.
		SDL_Rect card_local_rect = { 0, 0, card->rect.w, card->rect.h };
		SDL_RenderCopy(card->renderer, card->current, &card_local_rect,
			       &card->rect);
		return;
	}

	// Copy the upper current digit.
	// Card-local position for source.
	SDL_Rect half_source_rect = { 0, 0, card->rect.w, card->rect.h / 2 };
	SDL_Rect half_target_rect = { card->rect.x, card->rect.y, card->rect.w,
				      card->rect.h / 2 };
	SDL_RenderCopy(card->renderer, card->current, &half_source_rect,
		       &half_target_rect);

	// Copy the lower previous digit.
	half_source_rect.y = card->rect.h / 2;
	half_target_rect.y = card->rect.y + card->rect.h / 2;
	SDL_RenderCopy(card->renderer, card->previous, &half_source_rect,
		       &half_target_rect);

	/**
	 * Copy the flipping part.
	 * Upper half is previous and lower half is current.
	 * Just custom the destination Rect, zoom will be done automatically.
	 */
	bool upper_half = progress <= HALF_PROGRESS;
	double angle = upper_half ?
			       PI * progress / MAX_PROGRESS :
				     PI * (1.0 - (double)progress / MAX_PROGRESS);
	double scale = cos(angle);
	half_source_rect.y = upper_half ? 0 : card->rect.h / 2;
	half_target_rect.y =
		card->rect.y + (upper_half ?
					(double)card->rect.h / 2 * (1 - scale) :
					      (double)card->rect.h / 2);
	half_target_rect.h = (double)card->rect.h / 2 * scale;
	SDL_RenderCopy(card->renderer,
		       upper_half ? card->previous : card->current,
		       &half_source_rect, &half_target_rect);
}

void flipclock_card_destory(struct flipclock_card *card)
{
	if (card == NULL)
		return;
	_flipclock_card_close_fonts(card);
	_flipclock_card_destroy_textures(card);
	free(card);
}
