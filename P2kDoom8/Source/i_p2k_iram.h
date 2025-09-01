#ifndef I_P2K_IRAM_H
#define I_P2K_IRAM_H

#include "doomdef.h"
#include "doomtype.h"
#include "m_fixed.h"

//
// A vissprite_t is a thing that will be drawn during a refresh.
// i.e. a sprite object that is partly visible.
//

typedef struct vissprite_s
{
  int16_t x1, x2;
  fixed_t gx, gy;              // for line side calculation
  fixed_t gz;                   // global bottom for silhouette clipping
  fixed_t startfrac;           // horizontal position of x1
  fixed_t scale;
  fixed_t xiscale;             // negative if flipped
  fixed_t texturemid;
  uint16_t fracstep;

  int16_t lump_num;
  int16_t patch_topoffset;

  // for color translation and shadow draw, maxbright frames as well
  const uint8_t* colormap;

} vissprite_t;

typedef struct {
	uint8_t __far *_s_screen;

	const uint8_t* colormap;
	const uint8_t __far* source;
	uint8_t __far* dest;
} IRAM_GLOBALS_T;

#if defined(IRAM)
#define IRAM_Globals_Init ((void (*)(const IRAM_GLOBALS_T *)) (0x03fc43c0 | 1))

#define R_DrawColumnSprite ((void (*)(const draw_column_vars_t *)) (0x03fc43cc | 1))
#define R_DrawColumnFlat ((void (*)(uint8_t, const draw_column_vars_t *)) (0x03fc4804 | 1))

#define SlopeDiv ((int16_t (*)(uint32_t, uint32_t)) (0x03fc4aac | 1))
#define SlopeDiv16 ((int16_t (*)(uint16_t, uint16_t)) (0x03fc4adc | 1))

#define R_PointToAngle3 ((angle_t (*)(fixed_t, fixed_t)) (0x03fc4b0c | 1))
#define R_PointToAngle16 ((angle16_t (*)(fixed_t, fixed_t, int16_t, int16_t)) (0x03fc4ce0 | 1))

#define FixedMul ((fixed_t (*)(fixed_t, fixed_t)) (0x03fc4f18 | 1))
#define FixedMulAngle ((fixed_t (*)(fixed_t, fixed_t)) (0x03fc4f60 | 1))
#define FixedMul3216 ((fixed_t (*)(fixed_t, uint16_t)) (0x03fc4f84 | 1))
#define FixedApproxDiv ((fixed_t (*)(fixed_t, fixed_t)) (0x03fc4f98 | 1))

#define R_PointOnSide ((int8_t (*)(fixed_t, fixed_t, const __far mapnode_t *)) (0x03fc4fe0 | 1))
#define finesineapprox ((fixed_t (*)(int16_t)) (0x03fc505c | 1))
#define finecosineapprox ((fixed_t (*)(int16_t)) (0x03fc50b4 | 1))
#define R_PointToDist ((int16_t (*)(fixed_t, fixed_t, int16_t, int16_t)) (0x03fc5120 | 1))

#define R_GetColumn ((void (*)(const texture_t __far*, int16_t, int16_t *, int16_t *)) (0x03fc5230 | 1))
#define R_PointOnSegSide ((boolean (*)(fixed_t, fixed_t, const seg_t __far *)) (0x03fc529c | 1))
#define isort ((void (*)(vissprite_t **, int16_t)) (0x03fc5344 | 1))
#define Mod ((int16_t (*)(int16_t, int16_t)) (0x03fc53b8 | 1))

#define viewangletox ((uint8_t (*)(int16_t)) (0x03fc53ec | 1))
#endif

#if !defined(IRAM)
void IRAM_Globals_Init(const IRAM_GLOBALS_T *globals);
#endif

#endif /* !I_P2K_IRAM_H */
