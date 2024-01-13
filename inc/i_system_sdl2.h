#ifndef I_SYSTEM_SDL2_H
#define I_SYSTEM_SDL2_H

#include <SDL2/SDL.h>

#include "doomtype.h"

void I_FinishUpdate_e32(const byte* srcBuffer, const byte* pallete, const unsigned int width, const unsigned int height);

void I_SetPallete_e32(const byte* pallete);

unsigned short* I_GetBackBuffer();

unsigned short* I_GetFrontBuffer();

void I_Error (const char *error, ...);

void I_Quit_e32();

void I_InitScreen_e32();

void I_CreateBackBuffer_e32();

#endif // I_SYSTEM_SDL2_H
