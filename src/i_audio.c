/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
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
 *  System interface for sound.
 *
 *-----------------------------------------------------------------------------
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "z_zone.h"

#include "m_swap.h"
#include "i_sound.h"
#include "m_misc.h"
#include "w_wad.h"
#include "lprintf.h"
#include "s_sound.h"

#include "doomdef.h"
#include "doomstat.h"
#include "doomtype.h"

#include "d_main.h"

#include "m_fixed.h"

#include "global_data.h"

//#define __arm__

#ifdef GBA
#ifndef P2K
#include <gba.h>
#include <maxmod.h>    // Maxmod definitions for GBA


//These two files should be generated by MaxMod mmutil.
#include "../build/soundbank.h"
#include "../build/soundbank_bin.h"


typedef struct audio_map_t
{
    unsigned short doom_num;
    unsigned short mm_num;
}audio_map_t;


//Mapping between the Doom music num and the maxmod music number.
static const audio_map_t musicMap[NUMMUSIC] =
{
    {mus_None, 0},
    {mus_e1m1, MOD_D_E1M1},
    {mus_e1m2, MOD_D_E1M2},
    {mus_e1m3, MOD_D_E1M3},
    {mus_e1m4, MOD_D_E1M4},
    {mus_e1m5, MOD_D_E1M5},
    {mus_e1m6, MOD_D_E1M6},
    {mus_e1m7, MOD_D_E1M7},
    {mus_e1m8, MOD_D_E1M8},
    {mus_e1m9, MOD_D_E1M9},
    {mus_e2m1, MOD_D_E2M1},
    {mus_e2m2, MOD_D_E2M2},
    {mus_e2m3, MOD_D_E2M3},
    {mus_e2m4, MOD_D_E2M4},
    {mus_e2m5, MOD_D_E1M7},
    {mus_e2m6, MOD_D_E2M6},
    {mus_e2m7, MOD_D_E2M7},
    {mus_e2m8, MOD_D_E2M8},
    {mus_e2m9, MOD_D_E2M9},
    {mus_e3m1, MOD_D_E2M9},
    {mus_e3m2, MOD_D_E3M2},
    {mus_e3m3, MOD_D_E3M3},
    {mus_e3m4, MOD_D_E1M8},
    {mus_e3m5, MOD_D_E1M7},
    {mus_e3m6, MOD_D_E1M6},
    {mus_e3m7, MOD_D_E2M7},
    {mus_e3m8, MOD_D_E3M8},
    {mus_e3m9, MOD_D_E1M9},
    {mus_inter, MOD_D_INTER},
    {mus_intro, MOD_D_INTRO},
    {mus_bunny, MOD_D_BUNNY},
    {mus_victor, MOD_D_VICTOR},
    {mus_introa, MOD_D_INTROA},
    {mus_runnin, MOD_D_RUNNIN},
    {mus_stalks, MOD_D_STALKS},
    {mus_countd, MOD_D_COUNTD},
    {mus_betwee, MOD_D_BETWEE},
    {mus_doom, MOD_D_DOOM},
    {mus_the_da, MOD_D_THE_DA},
    {mus_shawn, MOD_D_SHAWN},
    {mus_ddtblu, MOD_D_DDTBLU},
    {mus_in_cit, MOD_D_IN_CIT},
    {mus_dead, MOD_D_DEAD},
    {mus_stlks2, MOD_D_STALKS},
    {mus_theda2, MOD_D_THE_DA},
    {mus_doom2, MOD_D_DOOM},
    {mus_ddtbl2, MOD_D_DDTBLU},
    {mus_runni2, MOD_D_RUNNIN},
    {mus_dead2, MOD_D_DEAD},
    {mus_stlks3, MOD_D_STALKS},
    {mus_romero, MOD_D_ROMERO},
    {mus_shawn2, MOD_D_SHAWN},
    {mus_messag, MOD_D_MESSAG},
    {mus_count2, MOD_D_COUNTD},
    {mus_ddtbl3, MOD_D_DDTBLU},
    {mus_ampie, MOD_D_AMPIE},
    {mus_theda3, MOD_D_THE_DA},
    {mus_adrian, MOD_D_ADRIAN},
    {mus_messg2, MOD_D_MESSAG},
    {mus_romer2, MOD_D_ROMERO},
    {mus_tense, MOD_D_TENSE},
    {mus_shawn3, MOD_D_SHAWN},
    {mus_openin, MOD_D_OPENIN},
    {mus_evil, MOD_D_EVIL},
    {mus_ultima, MOD_D_ULTIMA},
    {mus_read_m, MOD_D_READ_M},
    {mus_dm2ttl, MOD_D_DM2TTL},
    {mus_dm2int, MOD_D_DM2INT},
};

static const audio_map_t soundMap[NUMSFX] =
{
    {sfx_None, 0},
    {sfx_pistol, SFX_DSPISTOL},
    {sfx_shotgn, SFX_DSSHOTGN},
    {sfx_sgcock, SFX_DSSGCOCK},
    {sfx_dshtgn, SFX_DSDSHTGN},
    {sfx_dbopn, SFX_DSDBOPN},
    {sfx_dbcls, SFX_DSDBCLS},
    {sfx_dbload, SFX_DSDBLOAD},
    {sfx_plasma, SFX_DSPLASMA},
    {sfx_bfg, SFX_DSBFG},
    {sfx_sawup, SFX_DSSAWUP},
    {sfx_sawidl, SFX_DSSAWIDL},
    {sfx_sawful, SFX_DSSAWFUL},
    {sfx_sawhit, SFX_DSSAWHIT},
    {sfx_rlaunc, SFX_DSRLAUNC},
    {sfx_rxplod, SFX_DSRXPLOD},
    {sfx_firsht, SFX_DSFIRSHT},
    {sfx_firxpl, SFX_DSFIRXPL},
    {sfx_pstart, SFX_DSPSTART},
    {sfx_pstop, SFX_DSPSTOP},
    {sfx_doropn, SFX_DSDOROPN},
    {sfx_dorcls, SFX_DSDORCLS},
    {sfx_stnmov, SFX_DSSTNMOV},
    {sfx_swtchn, SFX_DSSWTCHN},
    {sfx_swtchx, SFX_DSSWTCHX},
    {sfx_plpain, SFX_DSPLPAIN},
    {sfx_dmpain, SFX_DSDMPAIN},
    {sfx_popain, SFX_DSPOPAIN},
    {sfx_vipain, SFX_DSVIPAIN},
    {sfx_mnpain, SFX_DSMNPAIN},
    {sfx_pepain, SFX_DSPEPAIN},
    {sfx_slop, SFX_DSSLOP},
    {sfx_itemup, SFX_DSITEMUP},
    {sfx_wpnup, SFX_DSWPNUP},
    {sfx_oof, SFX_DSOOF},
    {sfx_telept, SFX_DSTELEPT},
    {sfx_posit1, SFX_DSPOSIT1},
    {sfx_posit2, SFX_DSPOSIT2},
    {sfx_posit3, SFX_DSPOSIT3},
    {sfx_bgsit1, SFX_DSBGSIT1},
    {sfx_bgsit2, SFX_DSBGSIT2},
    {sfx_sgtsit, SFX_DSSGTSIT},
    {sfx_cacsit, SFX_DSCACSIT},
    {sfx_brssit, SFX_DSBRSSIT},
    {sfx_cybsit, SFX_DSCYBSIT},
    {sfx_spisit, SFX_DSSPISIT},
    {sfx_bspsit, SFX_DSBSPSIT},
    {sfx_kntsit, SFX_DSKNTSIT},
    {sfx_vilsit, SFX_DSVILSIT},
    {sfx_mansit, SFX_DSMANSIT},
    {sfx_pesit, SFX_DSPESIT},
    {sfx_sklatk, SFX_DSSKLATK},
    {sfx_sgtatk, SFX_DSSGTATK},
    {sfx_skepch, SFX_DSSKEPCH},
    {sfx_vilatk, SFX_DSVILATK},
    {sfx_claw, SFX_DSCLAW},
    {sfx_skeswg, SFX_DSSKESWG},
    {sfx_pldeth, SFX_DSPLDETH},
    {sfx_pdiehi, SFX_DSPDIEHI},
    {sfx_podth1, SFX_DSPODTH1},
    {sfx_podth2, SFX_DSPODTH2},
    {sfx_podth3, SFX_DSPODTH3},
    {sfx_bgdth1, SFX_DSBGDTH1},
    {sfx_bgdth2, SFX_DSBGDTH2},
    {sfx_sgtdth, SFX_DSSGTDTH},
    {sfx_cacdth, SFX_DSCACDTH},
    {sfx_skldth, SFX_DSSKLDTH},
    {sfx_brsdth, SFX_DSBRSDTH},
    {sfx_cybdth, SFX_DSCYBDTH},
    {sfx_spidth, SFX_DSSPIDTH},
    {sfx_bspdth, SFX_DSBSPDTH},
    {sfx_vildth, SFX_DSVILDTH},
    {sfx_kntdth, SFX_DSKNTDTH},
    {sfx_pedth, SFX_DSPEDTH},
    {sfx_skedth, SFX_DSSKEDTH},
    {sfx_posact, SFX_DSPOSACT},
    {sfx_bgact, SFX_DSBGACT},
    {sfx_dmact, SFX_DSDMACT},
    {sfx_bspact, SFX_DSBSPACT},
    {sfx_bspwlk, SFX_DSBSPWLK},
    {sfx_vilact, SFX_DSVILACT},
    {sfx_noway, SFX_DSNOWAY},
    {sfx_barexp, SFX_DSBAREXP},
    {sfx_punch, SFX_DSPUNCH},
    {sfx_hoof, SFX_DSHOOF},
    {sfx_metal, SFX_DSMETAL},
    {sfx_chgun, 0},
    {sfx_tink, SFX_DSTINK},
    {sfx_bdopn, SFX_DSBDOPN},
    {sfx_bdcls, SFX_DSBDCLS},
    {sfx_itmbk, SFX_DSITMBK},
    {sfx_flame, SFX_DSFLAME},
    {sfx_flamst, SFX_DSFLAMST},
    {sfx_getpow, SFX_DSGETPOW},
    {sfx_bospit, SFX_DSBOSPIT},
    {sfx_boscub, SFX_DSBOSCUB},
    {sfx_bossit, SFX_DSBOSSIT},
    {sfx_bospn, SFX_DSBOSPN},
    {sfx_bosdth, SFX_DSBOSDTH},
    {sfx_manatk, SFX_DSMANATK},
    {sfx_mandth, SFX_DSMANDTH},
    {sfx_sssit, SFX_DSSSSIT},
    {sfx_ssdth, SFX_DSSSDTH},
    {sfx_keenpn, SFX_DSKEENPN},
    {sfx_keendt, SFX_DSKEENDT},
    {sfx_skeact, SFX_DSSKEACT},
    {sfx_skesit, SFX_DSSKESIT},
    {sfx_skeatk, SFX_DSSKEATK},
    {sfx_radio, SFX_DSRADIO},
};
#endif
#endif

//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
// Returns a handle.
//
static int addsfx(int sfxid, int channel, int volume, int sep)
{
#ifdef GBA
#ifndef P2K
    int mmvol = volume * 4;

    if(mmvol > 255)
        mmvol = 255;

    mm_sound_effect sound;
    sound.id      = soundMap[sfxid].mm_num;
    sound.rate    = 1024;
    sound.handle  = 0;
    sound.volume  = mmvol;
    sound.panning = sep;

    mmEffectEx( &sound );
#endif
#endif

	return channel;
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
int I_StartSound(int id, int channel, int vol, int sep)
{
	if ((channel < 0) || (channel >= MAX_CHANNELS))
		return -1;

	// Returns a handle (not used).
    addsfx(id, channel, vol, sep);

	return channel;
}

//static SDL_AudioSpec audio;

void I_InitSound(void)
{

#ifdef GBA
#ifndef P2K
    mmInitDefault(soundbank_bin, 12);
#endif
#endif

	// Finished initialization.
    lprintf(LO_INFO,"I_InitSound: sound ready");
}

void I_PlaySong(int handle, int looping)
{
    if(handle == mus_None)
        return;

#ifdef GBA
#ifndef P2K
    mm_pmode mode = looping ? MM_PLAY_LOOP : MM_PLAY_ONCE;

    unsigned int song = musicMap[handle].mm_num;

    mmStart(song, mode);
#endif
#endif
}


void I_PauseSong (int handle)
{
#ifdef GBA
#ifndef P2K
    mmPause();
#endif
#endif
}

void I_ResumeSong (int handle)
{
#ifdef GBA
#ifndef P2K
    mmResume();
#endif
#endif
}

void I_StopSong(int handle)
{
#ifdef GBA
#ifndef P2K
    mmStop();
#endif
#endif
}

void I_SetMusicVolume(int volume)
{
#ifdef GBA
#ifndef P2K
    int mmvol = volume * 32;

    mmSetModuleVolume(mmvol);
#endif
#endif
}
