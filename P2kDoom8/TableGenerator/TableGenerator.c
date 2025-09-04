/*
 * Table Generator
 *
 * Compile:
 *   gcc TableGenerator.c -o TableGenerator
 *
 * Usage & Example:
 *   TableGenerator [VIEWWINDOWWIDTH]
 *   TableGenerator 60
 *
 * It will generate following lookup tables and constants used in graphics and rendering code:
 *  1. `xtoviewangleTable` table.
 *  2. `viewangletoxTable` table.
 *  3. `VIEWANGLETOXMAX` value.
 *  4. `screenheightarray` table.
 *  5. `negonearray` table.
 *
 * Copyright 2023-2025 by Frenkel Smeijers
 * Copyright 2025 EXL
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "FineTangentTable.h"

/*
#define VIEWWINDOWWIDTH                         (44)
#define viewwidth                               (VIEWWINDOWWIDTH)
*/
#define SCREENWIDTH                             (320)
#define FIELDOFVIEW                             (2048) /* fineangles in the SCREENWIDTH wide window. */
#define FRACBITS                                (16)
#define FRACUNIT                                (1L << FRACBITS)
#define FINEANGLES                              (8192)
#define ANGLETOFINESHIFT                        (19)
#define ANG90                                   (0x40000000)

typedef int32_t fixed_t;
typedef uint32_t angle_t;

static int32_t centerxfrac;
static int32_t viewangletox[FINEANGLES / 2];
static angle_t xtoviewangle[SCREENWIDTH + 1];

// TODO: Drop union
union int64_u {
	int64_t ll;
	struct {
		int16_t wl;
		fixed_t dw;
		int16_t wh;
	} __attribute__((packed)) s;
};

static inline fixed_t FixedMul(fixed_t a, fixed_t b) {
	union int64_u r;
	r.ll = (int64_t)a * b;
	return r.s.dw; // r.ll >> FRACBITS;
}

static inline fixed_t FixedDiv(fixed_t a, fixed_t b) {
	if (((uint32_t) labs(a) >> 14) >= (uint32_t) labs(b)) {
		return ((a ^ b) >> 31) ^ INT32_MAX;
	} else {
		union int64_u r;
		// r.ll = (int64_t)a << FRACBITS;
		r.s.wl = 0;
		r.s.dw = a;
		r.s.wh = (a < 0) ? 0xffff : 0x0000;
		return r.ll / b;
	}
}

int main (int argc, char *argv[]) {
	int32_t i;
	int32_t x;
	int32_t t;
	fixed_t focallength;

	if (argc != 2) {
		fprintf(stderr, "\n\n");
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "\tTableGenerator [VIEWWINDOWWIDTH]\n");
		fprintf(stderr, "\tTableGenerator 60\n");
		fprintf(stderr, "\n\n");
		return 1;
	}

	const int width = atoi(argv[1]);
	centerxfrac = ((int32_t) (width / 2)) << FRACBITS;

	//
	// use finetangent[] table to generate viewangletoxTable[]
	// viewangletoxTable[] will give the next greatest x after the view angle
	//
	// calc focallength so FIELDOFVIEW angles covers SCREENWIDTH
	focallength = FixedDiv(centerxfrac, finetangent[FINEANGLES / 4 + FIELDOFVIEW / 2]);
	for (i = 0; i < FINEANGLES / 2; ++i) {
		if (finetangent[i] > FRACUNIT * 2) {
			t = -1;
		} else if (finetangent[i] < -FRACUNIT * 2) {
			t = width + 1;
		} else {
			t = FixedMul(finetangent[i], focallength);
			t = (centerxfrac - t + FRACUNIT - 1) >> FRACBITS;
			if (t < -1) {
				t = -1;
			} else if (t > width + 1) {
				t = width + 1;
			}
		}
		viewangletox[i] = t;
	}

	fprintf(stdout, "\n\n");
	fprintf(stdout, "#define VIEWWINDOWWIDTH (%d)\n", width);

	fprintf(stdout, "\n\n");
	fprintf(stdout, "xtoviewangleTable[]\n");

	//
	// scan viewangletox[] to generate xtoviewangleTable[]
	// xtoviewangle will give the smallest view angle that maps to x
	//
	// FILE *fp1 = fopen("tabx2va.txt", "w");
	for (x = 0; x <= width; x++) {
		i = 0;

		while (viewangletox[i] > x) {
			i++;
		}

		xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANG90;

		fprintf(stdout, (((x + 1) % 6) ? "0x%04X, " : "0x%04X,\n"), xtoviewangle[x] / 0x10000);
	}

	fprintf(stdout, "\n\n");
	fprintf(stdout, "viewangletoxTable[]");

	//
	// take out the fencepost cases from viewangletox[]
	//
	// FILE* fp2 = fopen("tabva2x.txt", "w");
	int32_t centerx = width / 2;
	int32_t ov = -1;
	int32_t g_VIEWANGLETOXMAX = 0;
	for (i = 0; i < FINEANGLES / 2; i++) {
		t = FixedMul(finetangent[i], focallength);
		t = centerx - t;

		if (viewangletox[i] == -1) {
			viewangletox[i] = 0;
		} else if (viewangletox[i] == width + 1) {
			viewangletox[i]  = width;
		}

		if (viewangletox[i] == width) {
			g_VIEWANGLETOXMAX += 1;
		}

		if ((ov != viewangletox[i]) && (viewangletox[i] != width)) {
			fprintf(stdout, "\n");
			ov = viewangletox[i];
		}

		if ((viewangletox[i] > 0) && (viewangletox[i] != width)) {
			fprintf(stdout, "%d, ", viewangletox[i]);
		}
	}

	fprintf(stdout, "\n\n");
	fprintf(stdout, "#define VIEWANGLETOXMAX (%d)\n", g_VIEWANGLETOXMAX);

	fprintf(stdout, "\n\n");
	fprintf(stdout, "screenheightarray[]\n");
	for (i = 0; i < width; ++i) {
		fprintf(stdout, (((i + 1) % 6) ? "VIEWWINDOWHEIGHT, " : "VIEWWINDOWHEIGHT,\n"));
	}

	fprintf(stdout, "\n\n");
	fprintf(stdout, "negonearray[]\n");
	for (i = 0; i < width; ++i) {
		fprintf(stdout, (((i + 1) % 10) ? "-1, " : "-1,\n"));
	}

	fprintf(stdout, "\n\n");

	return 0;
}
