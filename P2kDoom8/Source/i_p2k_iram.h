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
#define ARM      (0)
#define THUMB    (1)

#define ADDR_IRAM_Globals_Init (0x03fc43c0 | THUMB)
#define ADDR_R_DrawColumnSprite (0x03fc43cc | THUMB)
#define ADDR_R_DrawColumnFlat (0x03fc4804 | THUMB)
#define ADDR_SlopeDiv (0x03fc4aac | THUMB)
#define ADDR_SlopeDiv16 (0x03fc4adc | THUMB)
#define ADDR_R_PointToAngle3 (0x03fc4b0c | THUMB)
#define ADDR_R_PointToAngle16 (0x03fc4ce0 | THUMB)
#define ADDR_FixedMul (0x03fc4f18 | THUMB)
#define ADDR_FixedMulAngle (0x03fc4f60 | THUMB)
#define ADDR_FixedMul3216 (0x03fc4f84 | THUMB)
#define ADDR_FixedApproxDiv (0x03fc4f98 | THUMB)
#define ADDR_R_PointOnSide (0x03fc4fe0 | THUMB)
#define ADDR_finesineapprox (0x03fc505c | THUMB)
#define ADDR_finecosineapprox (0x03fc50b4 | THUMB)
#define ADDR_R_PointToDist (0x03fc5120 | THUMB)
#define ADDR_R_GetColumn (0x03fc5230 | THUMB)
#define ADDR_R_PointOnSegSide (0x03fc529c | THUMB)
#define ADDR_isort (0x03fc5344 | THUMB)
#define ADDR_Mod (0x03fc53b8 | THUMB)
#define ADDR_viewangletox (0x03fc53ec | THUMB)

#define IRAM_Globals_Init ((void (*)(const IRAM_GLOBALS_T *)) (ADDR_IRAM_Globals_Init))
#define R_DrawColumnSprite ((void (*)(const draw_column_vars_t *)) (ADDR_R_DrawColumnSprite))
#define R_DrawColumnFlat ((void (*)(uint8_t, const draw_column_vars_t *)) (ADDR_R_DrawColumnFlat))
#define SlopeDiv ((int16_t (*)(uint32_t, uint32_t)) (ADDR_SlopeDiv))
#define SlopeDiv16 ((int16_t (*)(uint16_t, uint16_t)) (ADDR_SlopeDiv16))
#define R_PointToAngle3 ((angle_t (*)(fixed_t, fixed_t)) (ADDR_R_PointToAngle3))
#define R_PointToAngle16 ((angle16_t (*)(fixed_t, fixed_t, int16_t, int16_t)) (ADDR_R_PointToAngle16))
#define FixedMul ((fixed_t (*)(fixed_t, fixed_t)) (ADDR_FixedMul))
#define FixedMulAngle ((fixed_t (*)(fixed_t, fixed_t)) (ADDR_FixedMulAngle))
#define FixedMul3216 ((fixed_t (*)(fixed_t, uint16_t)) (ADDR_FixedMul3216))
#define FixedApproxDiv ((fixed_t (*)(fixed_t, fixed_t)) (ADDR_FixedApproxDiv))
#define R_PointOnSide ((int8_t (*)(fixed_t, fixed_t, const __far mapnode_t *)) (ADDR_R_PointOnSide))
#define finesineapprox ((fixed_t (*)(int16_t)) (ADDR_finesineapprox))
#define finecosineapprox ((fixed_t (*)(int16_t)) (ADDR_finecosineapprox))
#define R_PointToDist ((int16_t (*)(fixed_t, fixed_t, int16_t, int16_t)) (ADDR_R_PointToDist))
#define R_GetColumn ((void (*)(const texture_t __far*, int16_t, int16_t *, int16_t *)) (ADDR_R_GetColumn))
#define R_PointOnSegSide ((boolean (*)(fixed_t, fixed_t, const seg_t __far *)) (ADDR_R_PointOnSegSide))
#define isort ((void (*)(vissprite_t **, int16_t)) (ADDR_isort))
#define Mod ((int16_t (*)(int16_t, int16_t)) (ADDR_Mod))
#define viewangletox ((uint8_t (*)(int16_t)) (ADDR_viewangletox))
#endif

#if !defined(IRAM)
void IRAM_Globals_Init(const IRAM_GLOBALS_T *globals);
#endif

#endif /* !I_P2K_IRAM_H */
