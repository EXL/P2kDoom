/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2004 by
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
 *  DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
 *  plus functions to determine game mode (shareware, registered),
 *  parse command line parameters, configure game parameters (turbo),
 *  and call the startup functions.
 *
 *-----------------------------------------------------------------------------
 */



#if !defined(__P2K__)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#else
#include <filesystem.h>
#endif

#include "doomdef.h"
#include "doomtype.h"
#include "doomstat.h"
#include "d_net.h"
#include "dstrings.h"
#include "sounds.h"
#include "z_zone.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_video.h"
#include "f_finale.h"
#include "f_wipe.h"
#include "m_misc.h"
#include "m_menu.h"
#include "i_main.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "p_setup.h"
#include "r_draw.h"
#include "r_main.h"
#include "d_main.h"
#include "lprintf.h"  // jff 08/03/98 - declaration of lprintf
#include "am_map.h"
#include "m_cheat.h"

#include "doom_iwad.h"
#include "global_data.h"

void GetFirstMap(int *ep, int *map); // Ty 08/29/98 - add "-warp x" functionality
static void D_PageDrawer(void);
static void D_UpdateFPS(void);


// CPhipps - removed wadfiles[] stuff


//jff 1/22/98 parms for disabling music and sound
const boolean nosfxparm = false;
const boolean nomusicparm = false;

const skill_t startskill = sk_medium;
const int startepisode = 1;
const int startmap = 1;

const boolean nodrawers = false;

static const char* timedemo = NULL;//"demo1";

/*
 * D_PostEvent - Event handling
 *
 * Called by I/O functions when an event is received.
 * Try event handlers for each code area in turn.
 * cph - in the true spirit of the Boom source, let the 
 *  short ciruit operator madness begin!
 */

void D_PostEvent(event_t *ev)
{
    /* cph - suppress all input events at game start
   * FIXME: This is a lousy kludge */
    if (_g->gametic < 3)
        return;

    M_Responder(ev) ||
            (_g->gamestate == GS_LEVEL && (
                 C_Responder(ev) ||
                 ST_Responder(ev) ||
                 AM_Responder(ev)
                 )
             ) ||
            G_Responder(ev);

}

//
// D_AddFile
//
// Rewritten by Lee Killough
//
// Ty 08/29/98 - add source parm to indicate where this came from
// CPhipps - static, const char* parameter
//         - source is an enum
//         - modified to allocate & use new wadfiles array
void D_AddFile (const char *file, wad_source_t source)
{
//	char *gwa_filename=NULL;

	if (wadfiles != NULL)
		free(wadfiles);
	wadfiles = malloc(sizeof(*wadfiles)*(numwadfiles+1));

	wadfiles[numwadfiles].name =
		AddDefaultExtension(strcpy(malloc(strlen(file)+5), file), ".wad");
	wadfiles[numwadfiles].src = source; // Ty 08/29/98
	numwadfiles++;
	// proff: automatically try to add the gwa files
	// proff - moved from w_wad.c
//	gwa_filename=AddDefaultExtension(strcpy(malloc(strlen(file)+5), file), ".wad");
//	if (strlen(gwa_filename)>4)
//		if (!strcasecmp(gwa_filename+(strlen(gwa_filename)-4),".wad"))
//		{
//			char *ext;
//			ext = &gwa_filename[strlen(gwa_filename)-4];
//			ext[1] = 'g'; ext[2] = 'w'; ext[3] = 'a';

//			if (wadfiles!=NULL)
//				free(wadfiles);
//			wadfiles = malloc(sizeof(*wadfiles)*(numwadfiles+1));
//			wadfiles[numwadfiles].name = gwa_filename;
//			wadfiles[numwadfiles].src = source; // Ty 08/29/98
//			numwadfiles++;
//		}
}

//
// D_Wipe
//
// CPhipps - moved the screen wipe code from D_Display to here
// The screens to wipe between are already stored, this just does the timing
// and screen updating

static void D_Wipe(void)
{
    boolean done;
    int wipestart = I_GetTime () - 1;

    wipe_initMelt();

    do
    {
        int nowtime, tics;
        do
        {
            nowtime = I_GetTime();
            tics = nowtime - wipestart;
        } while (!tics);

        wipestart = nowtime;
        done = wipe_ScreenWipe(tics);

        I_UpdateNoBlit();
        M_Drawer();                   // menu is drawn even on top of wipes

    } while (!done);
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

static void D_Display (void)
{

    boolean wipe;
    boolean viewactive = false;

    if (nodrawers)                    // for comparative timing / profiling
        return;

    if (!I_StartDisplay())
        return;

    // save the current screen if about to wipe
    wipe = (_g->gamestate != _g->wipegamestate);

    if (wipe)
        wipe_StartScreen();

    if (_g->gamestate != GS_LEVEL) { // Not a level
        switch (_g->oldgamestate)
        {
            case -1:
            case GS_LEVEL:
                V_SetPalette(0); // cph - use default (basic) palette
            default:
                break;
        }

        switch (_g->gamestate)
        {
            case GS_INTERMISSION:
                WI_Drawer();
                break;
            case GS_FINALE:
                F_Drawer();
                break;
            case GS_DEMOSCREEN:
                D_PageDrawer();
                break;
            default:
                break;
        }
    }
    else if (_g->gametic != _g->basetic)
    { // In a level

        HU_Erase();

        // Work out if the player view is visible, and if there is a border
        viewactive = (!(_g->automapmode & am_active) || (_g->automapmode & am_overlay));

        // Now do the drawing
        if (viewactive)
            R_RenderPlayerView (&_g->player);

        if (_g->automapmode & am_active)
            AM_Drawer();

        ST_Drawer(true, false);

        HU_Drawer();
    }

    _g->oldgamestate = _g->wipegamestate = _g->gamestate;

    // menus go directly to the screen
    M_Drawer();          // menu is drawn even on top of everything

    D_BuildNewTiccmds();

    // normal update
    if (!wipe)
        I_FinishUpdate ();              // page flip or blit buffer
    else
    {
        // wipe update
        wipe_EndScreen();
        D_Wipe();
    }

    I_EndDisplay();
}

//
//  D_DoomLoop()
//
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//

void D_DoomStep(void)
{
	// frame syncronous IO operations

	I_StartFrame();

	// process one or more tics
	if (_g->singletics)
	{
		I_StartTic ();
		G_BuildTiccmd (&_g->netcmd);

		if (_g->advancedemo)
			D_DoAdvanceDemo ();

		M_Ticker ();
		G_Ticker ();

		_g->gametic++;
		_g->maketic++;
	}
	else
		TryRunTics (); // will run at least one tic

	// killough 3/16/98: change consoleplayer to displayplayer
	if (_g->player.mo) // cph 2002/08/10
		S_UpdateSounds(_g->player.mo);// move positional sounds

	// Update display, next frame, with current state.
	D_Display();

	_g->fps_show = 1;
	if(_g->fps_show)
	{
		D_UpdateFPS();
	}
}

static void D_DoomLoop(void)
{
    for (;;)
    {
        // frame syncronous IO operations

        I_StartFrame();

        // process one or more tics
        if (_g->singletics)
        {
            I_StartTic ();
            G_BuildTiccmd (&_g->netcmd);

            if (_g->advancedemo)
                D_DoAdvanceDemo ();

            M_Ticker ();
            G_Ticker ();

            _g->gametic++;
            _g->maketic++;
        }
        else
            TryRunTics (); // will run at least one tic

        // killough 3/16/98: change consoleplayer to displayplayer
        if (_g->player.mo) // cph 2002/08/10
            S_UpdateSounds(_g->player.mo);// move positional sounds

        // Update display, next frame, with current state.
        D_Display();


        if(_g->fps_show)
        {
            D_UpdateFPS();
        }
    }
}

static void D_UpdateFPS()
{
    _g->fps_frames++;

    unsigned int timenow = I_GetTime();
    if(timenow >= (_g->fps_timebefore + TICRATE))
    {
        unsigned int tics_elapsed = timenow - _g->fps_timebefore;
        fixed_t f_realfps = FixedDiv((_g->fps_frames*(TICRATE*10)) << FRACBITS, tics_elapsed <<FRACBITS);

        _g->fps_framerate = (f_realfps >> FRACBITS);

        _g->fps_frames = 0;
        _g->fps_timebefore = timenow;
    }
    else if(timenow < _g->fps_timebefore)
    {
        //timer overflow.
        _g->fps_timebefore = timenow;
        _g->fps_frames = 0;
    }
}

//
//  DEMO LOOP
//


//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker(void)
{
    if (--_g->pagetic < 0)
        D_AdvanceDemo();
}

//
// D_PageDrawer
//
static void D_PageDrawer(void)
{
    // proff/nicolas 09/14/98 -- now stretchs bitmaps to fullscreen!
    // CPhipps - updated for new patch drawing
    // proff - added M_DrawCredits
    if (_g->pagelump)
    {
        V_DrawNumPatch(0, 0, 0, _g->pagelump, CR_DEFAULT, VPT_STRETCH);
    }
}

//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
    _g->advancedemo = true;
}

/* killough 11/98: functions to perform demo sequences
 * cphipps 10/99: constness fixes
 */

static void D_SetPageName(const char *name)
{
    _g->pagelump = W_GetNumForName(name);
}

static void D_DrawTitle1(const char *name)
{
    S_StartMusic(mus_intro);
    _g->pagetic = (TICRATE*30);
    D_SetPageName(name);
}

static void D_DrawTitle2(const char *name)
{
    S_StartMusic(mus_dm2ttl);
    D_SetPageName(name);
}

/* killough 11/98: tabulate demo sequences
 */

static struct
{
    void (*func)(const char *);
    const char *name;
}

const demostates[][4] =
{
    {
        {D_DrawTitle1, "TITLEPIC"},
        {D_DrawTitle1, "TITLEPIC"},
        {D_DrawTitle2, "TITLEPIC"},
        {D_DrawTitle1, "TITLEPIC"},
    },

    {
        {G_DeferedPlayDemo, "demo1"},
        {G_DeferedPlayDemo, "demo1"},
        {G_DeferedPlayDemo, "demo1"},
        {G_DeferedPlayDemo, "demo1"},
    },
    {
        {D_SetPageName, "TITLEPIC"},
        {D_SetPageName, "TITLEPIC"},
        {D_SetPageName, "TITLEPIC"},
        {D_SetPageName, "TITLEPIC"},
    },

    {
        {G_DeferedPlayDemo, "demo2"},
        {G_DeferedPlayDemo, "demo2"},
        {G_DeferedPlayDemo, "demo2"},
        {G_DeferedPlayDemo, "demo2"},
    },

    {
        {D_SetPageName, "TITLEPIC"},
        {D_SetPageName, "TITLEPIC"},
        {D_SetPageName, "TITLEPIC"},
        {D_SetPageName, "TITLEPIC"},
    },

    {
        {G_DeferedPlayDemo, "demo3"},
        {G_DeferedPlayDemo, "demo3"},
        {G_DeferedPlayDemo, "demo3"},
        {G_DeferedPlayDemo, "demo3"},
    },

    {
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL},
    }


};

/*
 * This cycles through the demo sequences.
 * killough 11/98: made table-driven
 */

void D_DoAdvanceDemo(void)
{
    _g->player.playerstate = PST_LIVE;  /* not reborn */
    _g->advancedemo = _g->usergame = false;
    _g->gameaction = ga_nothing;

    _g->pagetic = TICRATE * 11;         /* killough 11/98: default behavior */
    _g->gamestate = GS_DEMOSCREEN;


    if (!demostates[++_g->demosequence][_g->gamemode].func)
        _g->demosequence = 0;

    demostates[_g->demosequence][_g->gamemode].func(demostates[_g->demosequence][_g->gamemode].name);
}

//
// D_StartTitle
//
void D_StartTitle (void)
{
    _g->gameaction = ga_nothing;
    _g->demosequence = -1;
    D_AdvanceDemo();
}

//
// CheckIWAD
//
// Verify a file is indeed tagged as an IWAD
// Scan its lumps for levelnames and return gamemode as indicated
// Detect missing wolf levels in DOOM II
//
// The filename to check is passed in iwadname, the gamemode detected is
// returned in gmode, hassec returns the presence of secret levels
//
// jff 4/19/98 Add routine to test IWAD for validity and determine
// the gamemode from it. Also note if DOOM II, whether secret levels exist
// CPhipps - const char* for iwadname, made static

static void CheckIWAD2(const char* iwadname, GameMode_t *gmode, boolean *hassec)
{
	wadinfo_t header;
#if !defined(__P2K__)
	FILE* fp = fopen(iwadname, "rb");
	fread(&header, sizeof(header), 1, fp);
#else
	UINT32 readen;
	WCHAR wpath[64];
	u_atou(iwadname, wpath);
	FILE_HANDLE_T fp = DL_FsOpenFile(wpath, FILE_READ_MODE, 0);
	DL_FsReadFile(&header, sizeof(header), 1, fp, &readen);
#endif
    int ud=0,rg=0,sw=0,cm=0,sc=0;

    if(!strncmp(header.identification, "IWAD", 4))
    {
		filelump_t *fileinfo;
        size_t length = header.numlumps;
		fileinfo = malloc(length*sizeof(filelump_t));

#if !defined(__P2K__)
		fseek (fp, header.infotableofs, SEEK_SET);
		fread (fileinfo, sizeof(filelump_t), length, fp);
		fclose(fp);
#else
		DL_FsFSeekFile(fp, header.infotableofs, SEEK_WHENCE_SET);
		DL_FsReadFile(fileinfo, sizeof(filelump_t), length, fp, &readen);
		DL_FsCloseFile(fp);
#endif

//        const filelump_t* fileinfo = (const filelump_t*)&iwad_data[header->infotableofs];

        while (length--)
        {
            if (fileinfo[length].name[0] == 'E' && fileinfo[length].name[2] == 'M' && fileinfo[length].name[4] == 0)
            {
              if (fileinfo[length].name[1] == '4')
                ++ud;
              else if (fileinfo[length].name[1] == '3')
                ++rg;
              else if (fileinfo[length].name[1] == '2')
                ++rg;
              else if (fileinfo[length].name[1] == '1')
                ++sw;
            }
            else if (fileinfo[length].name[0] == 'M' && fileinfo[length].name[1] == 'A' && fileinfo[length].name[2] == 'P' && fileinfo[length].name[5] == 0)
            {
              ++cm;
              if (fileinfo[length].name[3] == '3')
              {
                  if (fileinfo[length].name[4] == '1' || fileinfo[length].name[4] == '2')
                    ++sc;
              }
            }
			//Final Doom IWAD check hacks ~Kippykip
			//TNT - MURAL1
			else if (fileinfo[length].name[0] == 'M' && fileinfo[length].name[1] == 'U' && fileinfo[length].name[2] == 'R'  && fileinfo[length].name[3] == 'A' && fileinfo[length].name[4] == 'L' && fileinfo[length].name[5] == '1' && fileinfo[length].name[6] == 0)
            {
				*gmode = commercial;
				_g->gamemission = pack_tnt;
				_g->gamemode = commercial;
				return;
            }
			//Plutonia - WFALL1
			else if (fileinfo[length].name[0] == 'W' && fileinfo[length].name[1] == 'F' && fileinfo[length].name[2] == 'A'  && fileinfo[length].name[3] == 'L' && fileinfo[length].name[4] == 'L' && fileinfo[length].name[5] == '1' && fileinfo[length].name[6] == 0)
            {
				*gmode = commercial;
				_g->gamemission = pack_plut;
				_g->gamemode = commercial;
				return;
            }
        }

		free(fileinfo);
    }
    else
    {
        I_Error("CheckIWAD: IWAD tag not present");
    }

    // Determine game mode from levels present
    // Must be a full set for whichever mode is present
    // Lack of wolf-3d levels also detected here

    *gmode = indetermined;
    *hassec = false;
    if (cm>=30)
    {
        *gmode = commercial;
        *hassec = sc>=2;
    }
    else if (ud>=9)
        *gmode = retail;
    else if (rg>=18)
        *gmode = registered;
    else if (sw>=9)
        *gmode = shareware;
}

//
// IdentifyVersion
//
// Set the location of the defaults file and the savegame root
// Locate and validate an IWAD file
// Determine gamemode from the IWAD
//
// supports IWADs with custom names. Also allows the -iwad parameter to
// specify which iwad is being searched for if several exist in one dir.
// The -iwad parm may specify:
//
// 1) a specific pathname, which must exist (.wad optional)
// 2) or a directory, which must contain a standard IWAD,
// 3) or a filename, which must be found in one of the standard places:
//   a) current dir,
//   b) exe dir
//   c) $DOOMWADDIR
//   d) or $HOME
//
// jff 4/19/98 rewritten to use a more advanced search algorithm


static void IdentifyVersion()
{
#if !defined(__P2K__)
	const char *iwad_name = "doom1.wad";
#else
#if defined(EM1) || defined(EM2)
	const char *iwad_name = "/e/mobile/doom1.wad";
#else
	const char *iwad_name = "file://c/Elf/doom1.wad";
#endif
#endif
    CheckIWAD2(iwad_name, &_g->gamemode, &_g->haswolflevels);

    /* jff 8/23/98 set gamemission global appropriately in all cases
     * cphipps 12/1999 - no version output here, leave that to the caller
     */
    switch(_g->gamemode)
    {
        case retail:
        case registered:
        case shareware:
            _g->gamemission = doom;
            break;
        case commercial:
            _g->gamemission = doom2;
            break;

        default:
            _g->gamemission = none;
            break;
    }

    if (_g->gamemode == indetermined)
    {
        //jff 9/3/98 use logical output routine
        lprintf("%s\n","Unknown Game Version, may not work\n");
    }

	D_AddFile(iwad_name,source_iwad);
}

//
// D_DoomMainSetup
//
// CPhipps - the old contents of D_DoomMain, but moved out of the main
//  line of execution so its stack space can be freed

void D_DoomMainSetup(void)
{
#if defined(__P2K__)
    WCHAR wpath[64];
    UINT32 readen;

    lprintf("%s\n", "Loading recp_tab.bin...");

#if defined(EM1) || defined(EM2)
	u_atou("/a/elf/recp_tab.bin", wpath);
#else
	u_atou("file://c/Elf/recp_tab.bin", wpath);
#endif
	FILE_HANDLE_T fp1 = DL_FsOpenFile(wpath, FILE_READ_MODE, 0);
	reciprocalTable = NULL;
	reciprocalTable = AmMemAllocPointer(sizeof(unsigned int) * 65537);
	if (!reciprocalTable) {
		I_Error("=====================> Cannot allocate reciprocalTable HEAP: %d\n!", sizeof(unsigned int) * 65537);
	}
	DL_FsReadFile(reciprocalTable, sizeof(unsigned int) * 65537, 1, fp1, &readen);
	DL_FsCloseFile(fp1);
    lprintf("%s\n", "Loading math_tab.bin...");
#if defined(EM1) || defined(EM2)
    u_atou("/a/elf/math_tab.bin", wpath);
#else
    u_atou("file://c/Elf/math_tab.bin", wpath);
#endif
    FILE_HANDLE_T fp2 = DL_FsOpenFile(wpath, FILE_READ_MODE, 0);

	finetangent = suAllocMem(sizeof(fixed_t) * 4096, NULL);
	DL_FsReadFile(finetangent, sizeof(fixed_t) * 4096, 1, fp2, &readen);

	finesine = suAllocMem(sizeof(fixed_t) * 10240, NULL);
	DL_FsReadFile(finesine, sizeof(fixed_t) * 10240, 1, fp2, &readen);
    finecosine = &finesine[FINEANGLES/4];

    tantoangle = suAllocMem(sizeof(angle_t) * 2049, NULL);
    DL_FsReadFile(tantoangle, sizeof(angle_t) * 2049, 1, fp2, &readen);

    viewangletox = suAllocMem(sizeof(int) * 4096, NULL);
    DL_FsReadFile(viewangletox, sizeof(int) * 4096, 1, fp2, &readen);

	DL_FsCloseFile(fp2);
#else
    lprintf("%s\n", "Loading recp_tab.bin...");
    FILE *f = fopen("recp_tab.bin", "rb");
    reciprocalTable = malloc(sizeof(unsigned int) * 65537);
    fread(reciprocalTable, sizeof(unsigned int) * 65537, 1, f);
    fclose(f);

    lprintf("%s\n", "Loading math_tab.bin...");
    FILE *f2 = fopen("math_tab.bin", "rb");

    finetangent = malloc(sizeof(fixed_t) * 4096);
    fread(finetangent, sizeof(fixed_t) * 4096, 1, f2);

    finesine = malloc(sizeof(fixed_t) * 10240);
    fread(finesine, sizeof(fixed_t) * 10240, 1, f2);
    finecosine = &finesine[FINEANGLES/4];

    tantoangle = malloc(sizeof(angle_t) * 2049);
    fread(tantoangle, sizeof(angle_t) * 2049, 1, f2);

    viewangletox = malloc(sizeof(int) * 4096);
    fread(viewangletox, sizeof(int) * 4096, 1, f2);

    fclose(f2);
#endif
    IdentifyVersion();

    // jff 1/24/98 end of set to both working and command line value

    // CPhipps - localise title variable
    // print title for every printed line
    // cph - code cleaned and made smaller
    const char* doomverstr;

    switch ( _g->gamemode )
    {
        case retail:
            doomverstr = "The Ultimate DOOM";
            break;
        case shareware:
            doomverstr = "DOOM Shareware";
            break;
        case registered:
            doomverstr = "DOOM Registered";
            break;
        case commercial:  // Ty 08/27/98 - fixed gamemode vs gamemission
            switch (_g->gamemission)
            {
            case pack_plut:
                doomverstr = "DOOM 2: Plutonia Experiment";
                break;
            case pack_tnt:
                doomverstr = "DOOM 2: TNT - Evilution";
                break;
            default:
                doomverstr = "DOOM 2: Hell on Earth";
                break;
            }
            break;
        default:
            doomverstr = "Public DOOM";
            break;
    }

    /* cphipps - the main display. This shows the build date, copyright, and game type */

    lprintf("PrBoom (built %s)", version_date);
    lprintf("Playing: %s", doomverstr);
//    lprintf("%s\n", "PrBoom is released under the");
//    lprintf("%s\n", "GNU GPL v2.0.");

//    lprintf(LO_ALWAYS, "You are welcome to");
//    lprintf(LO_ALWAYS, "redistribute it under");
//    lprintf(LO_ALWAYS, "certain conditions.");

//    lprintf(LO_ALWAYS, "It comes with ABSOLUTELY\nNO WARRANTY.\nSee the file COPYING for\ndetails.");

//    lprintf(LO_ALWAYS, "\nPhew. Thats the nasty legal\nstuff out of the way.\nLets play Doom!\n");



    // init subsystems

    G_ReloadDefaults();    // killough 3/4/98: set defaults just loaded.
    // jff 3/24/98 this sets startskill if it was -1

    // CPhipps - move up netgame init
    //jff 9/3/98 use logical output routine
    lprintf("%s\n","D_InitNetGame.");
    D_InitNetGame();

    //jff 9/3/98 use logical output routine
    lprintf("%s\n","W_Init: Init WADfiles.");
    W_Init(); // CPhipps - handling of wadfiles init changed

    //jff 9/3/98 use logical output routine
    lprintf("%s\n","M_Init: Init misc info.");
    M_Init();

    //jff 9/3/98 use logical output routine
    lprintf("%s\n","R_Init: DOOM refresh daemon.");
    R_Init();

    //jff 9/3/98 use logical output routine
    lprintf("%s\n","P_Init: Init Playloop state.");
    P_Init();

    //jff 9/3/98 use logical output routine
    lprintf("%s\n","S_Init: Setting up sound.");
    S_Init(_g->snd_SfxVolume /* *8 */, _g->snd_MusicVolume /* *8*/ );

    //jff 9/3/98 use logical output routine
    lprintf("%s\n","HU_Init: Setting up HUD.");
    HU_Init();

    //jff 9/3/98 use logical output routine
    lprintf("%s\n","ST_Init: Init status bar.");
    ST_Init();

    lprintf("%s\n","G_LoadSettings: Loading settings.");
    G_LoadSettings();

//    _g->gamma = 2;
//    V_SetPalLump(_g->gamma);

    _g->idmusnum = -1; //jff 3/17/98 insure idmus number is blank

    _g->fps_show = false;

    _g->highDetail = false;

	lprintf("%s\n","INIT GRAPHICS START.");
    I_InitGraphics();

    if (timedemo)
    {
        _g->singletics = true;
        _g->timingdemo = true;            // show stats after quit
        G_DeferedPlayDemo(timedemo);
        _g->singledemo = true;            // quit after one demo
    }
    else
    {
		lprintf("%s\n","START TITLE!");
        D_StartTitle();                 // start up intro loop
    }
}

//
// D_DoomMain
//

void D_DoomMain(void)
{
    D_DoomMainSetup(); // CPhipps - setup out of main execution stack

    D_DoomLoop ();  // never returns
}

//
// GetFirstMap
//
// Ty 08/29/98 - determine first available map from the loaded wads and run it
//

void GetFirstMap(int *ep, int *map)
{
    int i,j; // used to generate map name
    boolean done = false;  // Ty 09/13/98 - to exit inner loops
    char test[6];  // MAPxx or ExMx plus terminator for testing
    char name[6];  // MAPxx or ExMx plus terminator for display
    boolean newlevel = false;  // Ty 10/04/98 - to test for new level
    int ix;  // index for lookup

    strcpy(name,""); // initialize
    if (*map == 0) // unknown so go search for first changed one
    {
        *ep = 1;
        *map = 1; // default E1M1 or MAP01
        if (_g->gamemode == commercial)
        {
            for (i=1;!done && i<33;i++)  // Ty 09/13/98 - add use of !done
            {
                sprintf(test,"MAP%02d",i);
                ix = W_CheckNumForName(test);
                if (ix != -1)  // Ty 10/04/98 avoid -1 subscript
                {
                        if (!*name)  // found one, not pwad.  First default.
                            strcpy(name,test);
                }
            }
        }
        else // one of the others
        {
            strcpy(name,"E1M1");  // Ty 10/04/98 - default for display
            for (i=1;!done && i<5;i++)  // Ty 09/13/98 - add use of !done
            {
                for (j=1;!done && j<10;j++)  // Ty 09/13/98 - add use of !done
                {
                    sprintf(test,"E%dM%d",i,j);
                    ix = W_CheckNumForName(test);
                    if (ix != -1)  // Ty 10/04/98 avoid -1 subscript
                    {

                            if (!*name)  // found one, not pwad.  First default.
                                strcpy(name,test);
                    }
                }
            }
        }
        //jff 9/3/98 use logical output routine
        lprintf("Auto-warping to first %slevel: %s\n",
                newlevel ? "new " : "", name);  // Ty 10/04/98 - new level test
    }
}
