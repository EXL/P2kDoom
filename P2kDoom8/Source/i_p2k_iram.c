#include <conio.h>
#include <dos.h>
#include <stdint.h>

#include "compiler.h"

#include "i_system.h"
#include "i_video.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"
#include "i_main.h"
#include "d_main.h"
#include "m_cheat.h"

#include "globdata.h"

#include "i_p2k_iram.h"

#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

// The screen is [SCREENWIDTH * SCREENHEIGHT];
static uint8_t __far* _s_screen;

//static const int16_t CENTERX = VIEWWINDOWWIDTH  / 2;
       const int16_t CENTERY = VIEWWINDOWHEIGHT / 2;

const uint8_t* colormap;

const uint8_t __far* source;
uint8_t __far* dest;

void IRAM_Globals_Init(const IRAM_GLOBALS_T *globals) {
	_s_screen = globals->screen;
}

inline static void R_DrawColumnPixel(uint8_t __far* dest, const byte __far* source, uint16_t frac)
{
#if VIEWWINDOWWIDTH == 60
	uint16_t color = colormap[source[frac>>COLBITS]];
	color = (color | (color << 8));

	uint16_t __far* d = (uint16_t __far*) dest;
	*d++ = color;
	*d   = color;
#elif VIEWWINDOWWIDTH == 120
	uint16_t color = colormap[source[frac>>COLBITS]];
	color = (color | (color << 8));

	uint16_t __far* d = (uint16_t __far*) dest;
	*d   = color;
#elif VIEWWINDOWWIDTH == 240
	*dest = colormap[source[frac>>COLBITS]];
#else
#error unsupported VIEWWINDOWWIDTH value
#endif
}

static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	int16_t l = count >> 4;
	while (l--)
	{
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;

		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;

		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;

		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case 14: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case 13: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case 12: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case 11: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case 10: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  9: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  8: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  7: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  6: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  5: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  4: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  3: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  2: R_DrawColumnPixel(dest, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  1: R_DrawColumnPixel(dest, source, frac);
	}
}

void R_DrawColumnSprite(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	source = dcvars->source;

	colormap = dcvars->colormap;

	dest = _s_screen + (dcvars->yl * SCREENWIDTH) + (dcvars->x * 4 * 60 / VIEWWINDOWWIDTH);

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	R_DrawColumn2(fracstep, frac, count);
}

static void R_DrawColumnFlat2(uint8_t col, uint8_t dontcare, int16_t count)
{
	UNUSED(dontcare);

	uint16_t color = col;
	color = (color << 8) | col;

	uint16_t __far* d = (uint16_t __far*)dest;

	uint16_t l = count >> 4;

	while (l--)
	{
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;

		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;

		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;

		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
		*d++ = color; *d = color; d += (SCREENWIDTH / 2) - 1;
	}

	switch (count & 15)
	{
		case 15: d[(SCREENWIDTH / 2) * 14] = color; d[(SCREENWIDTH / 2) * 14 + 1] = color;
		case 14: d[(SCREENWIDTH / 2) * 13] = color; d[(SCREENWIDTH / 2) * 13 + 1] = color;
		case 13: d[(SCREENWIDTH / 2) * 12] = color; d[(SCREENWIDTH / 2) * 12 + 1] = color;
		case 12: d[(SCREENWIDTH / 2) * 11] = color; d[(SCREENWIDTH / 2) * 11 + 1] = color;
		case 11: d[(SCREENWIDTH / 2) * 10] = color; d[(SCREENWIDTH / 2) * 10 + 1] = color;
		case 10: d[(SCREENWIDTH / 2) *  9] = color; d[(SCREENWIDTH / 2) *  9 + 1] = color;
		case  9: d[(SCREENWIDTH / 2) *  8] = color; d[(SCREENWIDTH / 2) *  8 + 1] = color;
		case  8: d[(SCREENWIDTH / 2) *  7] = color; d[(SCREENWIDTH / 2) *  7 + 1] = color;
		case  7: d[(SCREENWIDTH / 2) *  6] = color; d[(SCREENWIDTH / 2) *  6 + 1] = color;
		case  6: d[(SCREENWIDTH / 2) *  5] = color; d[(SCREENWIDTH / 2) *  5 + 1] = color;
		case  5: d[(SCREENWIDTH / 2) *  4] = color; d[(SCREENWIDTH / 2) *  4 + 1] = color;
		case  4: d[(SCREENWIDTH / 2) *  3] = color; d[(SCREENWIDTH / 2) *  3 + 1] = color;
		case  3: d[(SCREENWIDTH / 2) *  2] = color; d[(SCREENWIDTH / 2) *  2 + 1] = color;
		case  2: d[(SCREENWIDTH / 2) *  1] = color; d[(SCREENWIDTH / 2) *  1 + 1] = color;
		case  1: d[(SCREENWIDTH / 2) *  0] = color; d[(SCREENWIDTH / 2) *  0 + 1] = color;
	}
}

void R_DrawColumnFlat(uint8_t col, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	dest = _s_screen + (dcvars->yl * SCREENWIDTH) + (dcvars->x * 4 * 60 / VIEWWINDOWWIDTH);

	R_DrawColumnFlat2(col, col, count);
}
