#ifndef I_P2K_IRAM_H
#define I_P2K_IRAM_H

#include "doomtype.h"

typedef struct {
	uint8_t __far *screen;
} IRAM_GLOBALS_T;

#if defined(IRAM)
#define IRAM_Globals_Init ((void (*)(const IRAM_GLOBALS_T *)) (0x03fcf7f4 | 1))
#define R_DrawColumnSprite ((void (*)(const draw_column_vars_t *)) (0x03fcf800 | 1))
#define R_DrawColumnFlat ((void (*)(uint8_t, const draw_column_vars_t *)) (0x03fcfcb4 | 1))
#endif

#if !defined(IRAM)
void IRAM_Globals_Init(const IRAM_GLOBALS_T *globals);
#endif

#endif /* !I_P2K_IRAM_H */
