/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2001 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2023-2025 by
 *  Frenkel Smeijers
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
 *      Handles WAD file header, directory, lump I/O.
 *
 *-----------------------------------------------------------------------------
 */

// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdint.h>

#include "compiler.h"
#include "d_player.h"
#include "doomtype.h"
#include "i_system.h"

#include "w_wad.h"

#include "globdata.h"

#if defined(P2K)
#include <filesystem.h>
#endif

//#define BACKWARDS


//
// TYPES
//

typedef struct
{
  int32_t  filepos;
  uint16_t size;
  int16_t  filler;        // always zero
  char name[8];
} filelump_t;


//
// GLOBALS
//

#if !defined(P2K)
static FILE* fileWAD;

#if !defined WAD_FILE
#define WAD_FILE "DOOM1.WAD"
#endif

#else /* defined(P2K) */
static FILE_HANDLE_T fileWAD;

#if !defined WAD_FILE
#define WAD_FILE "P2kDoom8.wad"
#endif

#endif

static int16_t numlumps;

static filelump_t __far* fileinfo;

static void __far*__far* lumpcache;

//
// LUMP BASED ROUTINES.
//

#define BUFFERSIZE 512

#if !defined(P2K)
static void _ffread(void __far* ptr, uint16_t size, FILE* fp)
#else
static void _ffread(void __far* ptr, uint16_t size, FILE_HANDLE_T fp)
#endif
{
#if defined(P2K)
	UINT32 readen;
#endif
	uint8_t __far* dest = ptr;
	uint8_t buffer[BUFFERSIZE];

	while (size >= BUFFERSIZE)
	{
#if !defined(P2K)
		fread(buffer, BUFFERSIZE, 1, fp);
#else
		DL_FsReadFile(buffer, BUFFERSIZE, 1, fp, &readen);
#endif
		_fmemcpy(dest, buffer, BUFFERSIZE);
		dest += BUFFERSIZE;
		size -= BUFFERSIZE;
	}

	if (size > 0)
	{
#if !defined(P2K)
		fread(buffer, size, 1, fp);
#else
		DL_FsReadFile(buffer, size, 1, fp, &readen);
#endif
		_fmemcpy(dest, buffer, size);
	}
}

#if !defined(SDL) && !defined(P2K)
static boolean W_LoadWADIntoXMS(void)
{
#if !defined(P2K)
	fseek(fileWAD, 0, SEEK_END);
	int32_t size = ftell(fileWAD);
#else
	int32_t readen;

	DL_FsFSeekFile(fileWAD, 0, SEEK_WHENCE_END);
	int32_t size = DL_FsGetFileSize(fileWAD);
#endif

	boolean xms = Z_InitXms(size);
	if (!xms)
	{
		printf("Not enough XMS available\n");
		return false;
	}

	printf("Loading WAD into XMS\n");
	printf("Get Psyched!\n");

	uint8_t buffer[BUFFERSIZE];

#if !defined(P2K)
	fseek(fileWAD, 0, SEEK_SET);
#else
	DL_FsFSeekFile(fileWAD, 0, SEEK_WHENCE_SET);
#endif

	uint32_t dest = 0;

	while (size >= BUFFERSIZE)
	{
#if !defined(P2K)
		fread(buffer, BUFFERSIZE, 1, fileWAD);
#else
		DL_FsReadFile(buffer, BUFFERSIZE, 1, fileWAD, &readen);
#endif
		Z_MoveConventionalMemoryToExtendedMemory(dest, buffer, BUFFERSIZE);
		dest += BUFFERSIZE;
		size -= BUFFERSIZE;
	}

	if (size > 0)
	{
#if !defined(P2K)
		fread(buffer, size, 1, fileWAD);
#else
		DL_FsReadFile(buffer, size, 1, fileWAD, &readen);
#endif
		Z_MoveConventionalMemoryToExtendedMemory(dest, buffer, size);
	}

	return true;
}
#endif

static void W_ReadDataFromFile(void __far* dest, uint32_t src, uint16_t length)
{
#if !defined(P2K)
	fseek(fileWAD, src, SEEK_SET);
#else
	DL_FsFSeekFile(fileWAD, src, SEEK_WHENCE_SET);
#endif
	_ffread(dest, length, fileWAD);
}


typedef void (*W_ReadData_f)(void __far* dest, uint32_t src, uint16_t length);
static W_ReadData_f readfunc;


typedef struct
{
  char identification[4]; // Should be "IWAD" or "PWAD".
  int16_t  numlumps;
  int16_t  filler;        // always zero
  int32_t  infotableofs;
} wadinfo_t;


void W_Init(void)
{
	printf("\tadding " WAD_FILE "\n");
	printf("\tshareware version.\n");

#if !defined(P2K)
	fileWAD = fopen(WAD_FILE, "rb");
	if (fileWAD == NULL)
		I_Error("Can't open " WAD_FILE ".");
#else
	fileWAD = DL_FsOpenFile(g_res_file_path_ptr, FILE_READ_MODE, 0);
#endif

#if !defined(SDL) && !defined(P2K)
	boolean xms = W_LoadWADIntoXMS();
#else
	boolean xms = 0;
#endif

	readfunc = xms ? Z_MoveExtendedMemoryToConventionalMemory : W_ReadDataFromFile;

	wadinfo_t header;
	readfunc(&header, 0, sizeof(header));

	fileinfo = Z_MallocStatic(header.numlumps * sizeof(filelump_t));
	readfunc(fileinfo, header.infotableofs, sizeof(filelump_t) * header.numlumps);

	lumpcache = Z_MallocStatic(header.numlumps * sizeof(*lumpcache));
	_fmemset(lumpcache, 0, header.numlumps * sizeof(*lumpcache));

	numlumps = header.numlumps;
}


void W_Shutdown(void)
{
	readfunc = W_ReadDataFromFile;

#if defined(P2K)
	DL_FsCloseFile(fileWAD);
#endif
}


const char __far* PUREFUNC W_GetNameForNum(int16_t num)
{
	return fileinfo[num].name;
}


//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//

uint16_t PUREFUNC W_LumpLength(int16_t num)
{
	return fileinfo[num].size;
}


// W_GetNumForName
// bombs out if not found.
//
int16_t PUREFUNC W_GetNumForName(const char *name)
{
	char name8[8];
	strncpy(name8, name, sizeof(name8));

#if BACKWARDS
	for (int16_t i = numlumps - 1; i >= 0; i--)
#else
	for (int16_t i = 0; i < numlumps; i++)
#endif
	{
		if (Z_EqualNames(fileinfo[i].name, name8))
		{
			return i;
		}
	}

	I_Error("W_GetNumForName: %.8s not found", name);
	return -1;
}


void W_ReadLumpByNum(int16_t num, void __far* ptr)
{
	const filelump_t __far* lump = &fileinfo[num];
	readfunc(ptr, lump->filepos, lump->size);
}


const void __far* PUREFUNC W_GetLumpByNumAutoFree(int16_t num)
{
	const filelump_t __far* lump = &fileinfo[num];

	void __far* ptr = Z_MallocLevel(lump->size, NULL);

	readfunc(ptr, lump->filepos, lump->size);
	return ptr;
}


static void __far* PUREFUNC W_GetLumpByNumWithUser(int16_t num, void __far*__far* user)
{
	const filelump_t __far* lump = &fileinfo[num];

	void __far* ptr = Z_MallocStaticWithUser(lump->size, user);

	readfunc(ptr, lump->filepos, lump->size);
	return ptr;
}


int16_t W_GetFirstInt16(int16_t num)
{
	const filelump_t __far* lump = &fileinfo[num];

	int16_t firstInt16;

	readfunc(&firstInt16, lump->filepos, sizeof(int16_t));
	return firstInt16;
}


const void __far* PUREFUNC W_GetLumpByNum(int16_t num)
{
	if (lumpcache[num])
		Z_ChangeTagToStatic(lumpcache[num]);
	else
		lumpcache[num] = W_GetLumpByNumWithUser(num, &lumpcache[num]);

	return lumpcache[num];
}


boolean PUREFUNC W_IsLumpCached(int16_t num)
{
	return lumpcache[num] != NULL;
}


const void __far* PUREFUNC W_TryGetLumpByNum(int16_t num)
{
	if (lumpcache[num])
	{
		Z_ChangeTagToStatic(lumpcache[num]);
		return lumpcache[num];
	}
	else if (Z_IsEnoughFreeMemory(W_LumpLength(num)))
		return W_GetLumpByNum(num);
	else
		return NULL;
}
