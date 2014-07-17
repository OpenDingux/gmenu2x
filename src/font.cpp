#include "font.h"

#include "debug.h"
#include "surface.h"
#include "utilities.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>

/* TODO: Let the theme choose the font and font size */
#define TTF_FONT "/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed.ttf"
#define TTF_FONT_SIZE 12

using namespace std;

Font *Font::defaultFont()
{
	return new Font(TTF_FONT, TTF_FONT_SIZE);
}

Font::Font(const std::string &path, unsigned int size)
{
	font = nullptr;
	lineSpacing = 1;

	/* Note: TTF_Init and TTF_Quit perform reference counting, so call them
	 * both unconditionally for each font. */
	if (TTF_Init() < 0) {
		ERROR("Unable to init SDL_ttf library\n");
		return;
	}

	font = TTF_OpenFont(path.c_str(), size);
	if (!font) {
		ERROR("Unable to open font\n");
		TTF_Quit();
		return;
	}

	lineSpacing = TTF_FontLineSkip(font);
}

Font::~Font()
{
	if (font) {
		TTF_CloseFont(font);
		TTF_Quit();
	}
}

int Font::getTextWidth(const char *text)
{
	if (font) {
		int w, h;
		TTF_SizeUTF8(font, text, &w, &h);
		return w;
	}
	else return 1;
}

void Font::write(Surface *surface, std::string const& text,
				int x, int y, HAlign halign, VAlign valign)
{
	if (!font) {
		return;
	}
	if (text.empty()) {
		// SDL_ttf will return a nullptr when rendering the empty string.
		return;
	}

	switch (valign) {
	case VAlignTop:
		break;
	case VAlignMiddle:
		y -= lineSpacing / 2;
		break;
	case VAlignBottom:
		y -= lineSpacing;
		break;
	}

	SDL_Color color = { 0, 0, 0, 0 };
	SDL_Surface *s = TTF_RenderUTF8_Blended(font, text.c_str(), color);
	if (!s) {
		ERROR("Font rendering failed for text \"%s\"\n", text.c_str());
		return;
	}

	switch (halign) {
	case HAlignLeft:
		break;
	case HAlignCenter:
		x -= s->w / 2;
		break;
	case HAlignRight:
		x -= s->w;
		break;
	}

	SDL_Rect rect = { (Sint16) x, (Sint16) (y - 1), 0, 0 };
	SDL_BlitSurface(s, NULL, surface->raw, &rect);

	/* Note: rect.x / rect.y are reset everytime because SDL_BlitSurface
	 * will modify them if negative */
	rect.x = x;
	rect.y = y + 1;
	SDL_BlitSurface(s, NULL, surface->raw, &rect);

	rect.x = x - 1;
	rect.y = y;
	SDL_BlitSurface(s, NULL, surface->raw, &rect);

	rect.x = x + 1;
	rect.y = y;
	SDL_BlitSurface(s, NULL, surface->raw, &rect);
	SDL_FreeSurface(s);

	rect.x = x;
	rect.y = y;
	color.r = 0xff;
	color.g = 0xff;
	color.b = 0xff;

	s = TTF_RenderUTF8_Blended(font, text.c_str(), color);
	if (!s) {
		ERROR("Font rendering failed for text \"%s\"\n", text.c_str());
		return;
	}
	SDL_BlitSurface(s, NULL, surface->raw, &rect);
	SDL_FreeSurface(s);
}