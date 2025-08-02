/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2023-2025 Frenkel Smeijers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      DOS implementation of i_system.h
 *
 *-----------------------------------------------------------------------------*/

#if defined(SDL)

#if defined(SDL2)
#include <SDL2/SDL.h>
#endif

#elif defined(P2K)
#include <time_date.h>
#if defined(EP1) || defined(EP2)
#include <stdargs.h>
#else
#include <stdarg.h>
#endif
#else
#include <conio.h>
#include <dos.h>
#endif

#if !defined(P2K)
#include <stdarg.h>
#include <time.h>
#endif

#include "doomdef.h"
#include "doomtype.h"
#include "compiler.h"
#include "a_taskmn.h"
#include "d_main.h"
#include "i_system.h"
#include "globdata.h"
#include "m_cheat.h"

void I_InitGraphicsHardwareSpecificCode(void);
void I_ShutdownGraphics(void);


static boolean isGraphicsModeSet = false;


//**************************************************************************************
//
// Screen code
//

void I_SetScreenMode(uint16_t mode)
{
	UNUSED(mode);
#if !defined(SDL) && !defined(P2K)
	union REGS regs;
	regs.w.ax = mode;
	int86(0x10, &regs, &regs);
#endif
}


void I_InitGraphics(void)
{
	I_InitGraphicsHardwareSpecificCode();
	isGraphicsModeSet = true;
}


//**************************************************************************************
//
// Keyboard code
//

#if !defined(SDL) && !defined(P2K)
#define KEYBOARDINT 9
#define KBDQUESIZE 32
static byte keyboardqueue[KBDQUESIZE];
static int16_t kbdtail, kbdhead;
static boolean isKeyboardIsrSet = false;

#if defined __DJGPP__ 
static _go32_dpmi_seginfo oldkeyboardisr, newkeyboardisr;
#else
static void __interrupt __far (*oldkeyboardisr)(void);
#endif

static void __interrupt __far I_KeyboardISR(void)	
{
	// Get the scan code
	keyboardqueue[kbdhead & (KBDQUESIZE - 1)] = inp(0x60);
	kbdhead++;

	// Tell the XT keyboard controller to clear the key
	byte temp;
	outp(0x61, (temp = inp(0x61)) | 0x80);
	outp(0x61, temp);

	// acknowledge the interrupt
	outp(0x20, 0x20);
}
#endif


void I_InitKeyboard(void)
{
#if !defined(SDL) && !defined(P2K)
	replaceInterrupt(oldkeyboardisr, newkeyboardisr, KEYBOARDINT, I_KeyboardISR);
	isKeyboardIsrSet = true;
#endif
}


#define SC_ESCAPE			0x01
#define SC_MINUS			0x0c
#define SC_PLUS				0x0d
#define SC_TAB				0x0f
#define SC_BRACKET_LEFT		0x1a
#define SC_BRACKET_RIGHT	0x1b
#define SC_ENTER			0x1c
#define SC_CTRL				0x1d
#define SC_LSHIFT			0x2a
#define SC_RSHIFT			0x36
#define SC_COMMA			0x33
#define SC_PERIOD			0x34
#define SC_ALT				0x38
#define SC_SPACE			0x39
#define SC_F10				0x44
#define SC_UPARROW			0x48
#define SC_DOWNARROW		0x50
#define SC_LEFTARROW		0x4b
#define SC_RIGHTARROW		0x4d

#define SC_Q	0x10
#define SC_P	0x19
#define SC_A	0x1e
#define SC_L	0x26
#define SC_Z	0x2c
#define SC_M	0x32


#if defined VIDEO_MODE_CGA
#define SC_F5				0x3f
void I_SwitchPalette(void);
#endif

#if defined(SDL)
static evtype_t modkey_state = ev_keyup;
#endif

void I_StartTic(void)
{
	//
	// process keyboard events
	//
#if defined(SDL)
	event_t ev;
	SDL_Event event;
	ev.data1 = 0;

	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				ev.type = (event.type == SDL_KEYUP) ? ev_keyup : ev_keydown;
				switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_Q:
						ev.data1 = KEYD_START;
						break;
					case SDL_SCANCODE_X:
						if (modkey_state == ev_keyup) {
							ev.data1 = KEYD_A;
						} else {
							if (ev.type == ev_keydown) {
								Apply_Cheat(CHEAT_IDKFA_GIVE_ALL);
							}
						}
						break;
					case SDL_SCANCODE_UP:
						ev.data1 = KEYD_UP;
						break;
					case SDL_SCANCODE_DOWN:
						ev.data1 = KEYD_DOWN;
						break;
					case SDL_SCANCODE_LEFT:
						ev.data1 = KEYD_LEFT;
						break;
					case SDL_SCANCODE_RIGHT:
						ev.data1 = KEYD_RIGHT;
						break;
					case SDL_SCANCODE_W:
						if (modkey_state == ev_keyup) {
							ev.data1 = KEYD_SELECT;
						} else {
							if (ev.type == ev_keydown) {
								Apply_Cheat(CHEAT_ROCKETS_ENABLE);
							}
						}
						break;
					case SDL_SCANCODE_Z:
						if (modkey_state == ev_keyup) {
							ev.data1 = KEYD_B;
						} else {
							if (ev.type == ev_keydown) {
								Apply_Cheat(CHEAT_IDDQD_GOD);
							}
						}
						break;
					case SDL_SCANCODE_A:
						if (modkey_state == ev_keyup) {
							ev.data1 = KEYD_L;
						} else {
							if (ev.type == ev_keydown) {
								Apply_Cheat(CHEAT_CHOPPERS_CHAINSAW);
							}
						}
						break;
					case SDL_SCANCODE_S:
						if (modkey_state == ev_keyup) {
							ev.data1 = KEYD_R;
						} else {
							if (ev.type == ev_keydown) {
								Apply_Cheat(CHEAT_IDRATE_FPS);
							}
						}
						break;
					case SDL_SCANCODE_1:
						ev.data1 = KEYD_BRACKET_LEFT;
						break;
					case SDL_SCANCODE_2:
						ev.data1 = KEYD_BRACKET_RIGHT;
						break;
					case SDL_SCANCODE_C:
						modkey_state = ev.type;
						ev.data1 = KEYD_STRAFE;
						break;
					default:
						break;
				}
				if(ev.data1 != 0)
					D_PostEvent(&ev);
				break;
			case SDL_QUIT:
				I_Quit();
				return;
			default:
				break;
		}
	}
#elif defined(P2K)
	/* TODO */
#else
	byte k;
	event_t ev;

	while (kbdtail < kbdhead)
	{
		k = keyboardqueue[kbdtail & (KBDQUESIZE - 1)];
		kbdtail++;

		// extended keyboard shift key bullshit
		if ((k & 0x7f) == SC_LSHIFT || (k & 0x7f) == SC_RSHIFT)
		{
			if (keyboardqueue[(kbdtail - 2) & (KBDQUESIZE - 1)] == 0xe0)
				continue;
			k &= 0x80;
			k |= SC_RSHIFT;
		}

		if (k == 0xe0)
			continue;               // special / pause keys
		if (keyboardqueue[(kbdtail - 2) & (KBDQUESIZE - 1)] == 0xe1)
			continue;                               // pause key bullshit

		if (k == 0xc5 && keyboardqueue[(kbdtail - 2) & (KBDQUESIZE - 1)] == 0x9d)
		{
			//ev.type  = ev_keydown;
			//ev.data1 = KEY_PAUSE;
			//D_PostEvent(&ev);
			continue;
		}

		if (k & 0x80)
			ev.type = ev_keyup;
		else
			ev.type = ev_keydown;

		k &= 0x7f;
		switch (k)
		{
			case SC_ESCAPE:
				ev.data1 = KEYD_START;
				break;
			case SC_ENTER:
			case SC_SPACE:
				ev.data1 = KEYD_A;
				break;
			case SC_RSHIFT:
				ev.data1 = KEYD_SPEED;
				break;
			case SC_UPARROW:
				ev.data1 = KEYD_UP;
				break;
			case SC_DOWNARROW:
				ev.data1 = KEYD_DOWN;
				break;
			case SC_LEFTARROW:
				ev.data1 = KEYD_LEFT;
				break;
			case SC_RIGHTARROW:
				ev.data1 = KEYD_RIGHT;
				break;
			case SC_TAB:
				ev.data1 = KEYD_SELECT;
				break;
			case SC_CTRL:
				ev.data1 = KEYD_B;
				break;
			case SC_ALT:
				ev.data1 = KEYD_STRAFE;
				break;
			case SC_COMMA:
				ev.data1 = KEYD_L;
				break;
			case SC_PERIOD:
				ev.data1 = KEYD_R;
				break;
			case SC_MINUS:
				ev.data1 = KEYD_MINUS;
				break;
			case SC_PLUS:
				ev.data1 = KEYD_PLUS;
				break;
			case SC_BRACKET_LEFT:
				ev.data1 = KEYD_BRACKET_LEFT;
				break;
			case SC_BRACKET_RIGHT:
				ev.data1 = KEYD_BRACKET_RIGHT;
				break;

#if defined VIDEO_MODE_CGA
			case SC_F5:
				if (ev.type == ev_keydown)
					I_SwitchPalette();
				continue;
#endif

			case SC_F10:
				I_Quit();
			default:
				if (SC_Q <= k && k <= SC_P)
				{
					ev.data1 = "qwertyuiop"[k - SC_Q];
					break;
				}
				else if (SC_A <= k && k <= SC_L)
				{
					ev.data1 = "asdfghjkl"[k - SC_A];
					break;
				}
				else if (SC_Z <= k && k <= SC_M)
				{
					ev.data1 = "zxcvbnm"[k - SC_Z];
					break;
				}
				else
					continue;
		}
		D_PostEvent(&ev);
	}
#endif
}


//**************************************************************************************
//
// Returns time in 1/35th second tics.
//

#if !defined(SDL) && !defined(P2K)
#define TIMER_PRIORITY 0

static volatile int32_t ticcount;

static boolean isTimerSet;


static void I_TimerISR(void)
{
	ticcount++;
}
#endif

int32_t I_GetTime(void)
{
#if !defined(SDL) && !defined(P2K)
    return ticcount;
#else

#if defined(SDL)
    Uint32 ms = SDL_GetTicks();

    int thistimereply = ms * TICRATE / 1000;

    return thistimereply;
#endif

#if defined(P2K)
    #define TICKS_PER_SEC                  (8192)

    int thistimereply = ((uint32_t) suPalReadTime()) * TICRATE / TICKS_PER_SEC;

    return thistimereply;
#endif

#endif
}


void I_InitTimer(void)
{
#if !defined(SDL) && !defined(P2K)
	TS_ScheduleTask(I_TimerISR, TICRATE, TIMER_PRIORITY);

	isTimerSet = true;
#endif
}

#if !defined(SDL) && !defined(P2K)
static void I_ShutdownTimer(void)
{
	TS_Terminate(TIMER_PRIORITY);
	TS_Shutdown();
}
#endif

//**************************************************************************************
//
// Exit code
//

static void I_Shutdown(void)
{
	if (isGraphicsModeSet)
		I_ShutdownGraphics();

	I_ShutdownSound();

#if !defined(SDL) && !defined(P2K)
	if (isTimerSet)
		I_ShutdownTimer();

	if (isKeyboardIsrSet)
	{
		restoreInterrupt(KEYBOARDINT, oldkeyboardisr, newkeyboardisr);
	}
#endif

	W_Shutdown();
	Z_Shutdown();
}


#if !defined(SDL) && !defined(P2K)
segment_t I_GetTextModeVideoMemorySegment(void);
#endif

void I_Quit(void)
{
	I_Shutdown();

#if !defined(SDL) && !defined(P2K)
	int16_t lumpnum = W_GetNumForName("ENDOOM");
	W_ReadLumpByNum(lumpnum, D_MK_FP(I_GetTextModeVideoMemorySegment(), 0 + __djgpp_conventional_base));

	union REGS regs;
	regs.h.ah = 2;
	regs.h.bh = 0;
	regs.h.dl = 0;
	regs.h.dh = 23;
	int86(0x10, &regs, &regs);

	printf("\n");
#endif

#if defined(P2K)
	I_Quit_P2k();
#else
	exit(0);
#endif
}

#if !defined(P2K)
void I_Error (const char *error, ...)
{
	va_list argptr;

	I_Shutdown();

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}
#endif
