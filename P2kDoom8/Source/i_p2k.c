/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2023-2025 Frenkel Smeijers
 *  Copyright (C) 2025 EXL
 *  Copyright (C) 2025 fkcoder / Siesta
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
 *      Video code for VGA Mode 13h 320x200 256 colors
 *      Effective resolutions  60x128
 *                            120x128
 *                            240x128
 *
 * EXL, 04-Aug-2025:
 *      P2K platfrom implementation based on i_vvga13.c blitter by FrenkelS.
 *
 *-----------------------------------------------------------------------------*/

#include <conio.h>
#include <dos.h>
#include <stdint.h>

#include <loader.h>
#include <apps.h>
#include <dl.h>
#include <dl_keypad.h>
#include <dal.h>
#include <filesystem.h>
#include <uis.h>
#include <canvas.h>
#include <mem.h>
#include <time_date.h>
#include <utilities.h>

#if defined(FTR_GFX_ATI)
#include <ati.h>
#elif defined(FTR_GFX_NVIDIA)
#include <nvidia.h>
#endif

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

#if defined(FPS)
#define TIMER_FAST_UPDATE_MS (1000 / FPS) /* Dynamic FPS based on -DFPS flag. */
#else
#error "FPS not defined! Please compile it with -DFPS=<value> (e.g., -DFPS=30)."
#endif

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_EXIT = 0xE398,
	APP_TIMER_LOOP
} APP_TIMER_T;

#if defined(FTR_GFX_ATI)
typedef struct {
	AHIDRVINFO_T *info_driver;
	AHIDEVCONTEXT_T context;
	AHISURFACE_T screen;
	AHISURFACE_T draw;
	AHISURFINFO_T info_surface_screen;
	AHISURFINFO_T info_surface_draw;
	AHIBITMAP_T bitmap;

	AHIPOINT_T point_bitmap;
	AHIRECT_T rect_bitmap;
	AHIRECT_T rect_draw;
	AHIUPDATEPARAMS_T update_params;
} APP_AHI_T;
#elif defined(FTR_GFX_NVIDIA)
typedef struct {
	GF_HANDLE gxHandle;
	GXRECT fb0_rect;
	UINT8 *fb0;
} APP_GFSDK_T;
#elif defined(FTR_GFX_DAL)
typedef struct {
	UINT8 *bitmap;
	GRAPHIC_REGION_T draw_region;
} APP_DAL_T;
#endif

typedef struct {
	UINT32 pressed;
	UINT32 released;
} APP_KEYBOARD_T;

typedef struct {
	APPLICATION_T app;

	BOOL is_CSTN_display;
	UINT16 width;
	UINT16 height;
	UINT16 bmp_width;
	UINT16 bmp_height;

	UINT8 *p_bitmap;

#if defined(FTR_GFX_ATI)
	APP_AHI_T ahi;
#elif defined(FTR_GFX_NVIDIA)
	APP_GFSDK_T gfsdk;
#elif defined(FTR_GFX_DAL)
	APP_DAL_T dal;
#endif

	APP_KEYBOARD_T keys;
	UINT32 timer_handle;
	UINT8 keyboard_volume_level;
} APP_INSTANCE_T;

#if defined(EP1)
UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments);                                /* ElfPack 2.x entry point. */
#elif defined(EM1)
int _main(ElfLoaderApp ela);                                                 /* ElfPack 1.x M*CORE entry point. */
#elif defined(EM2)
UINT32 ELF_Entry(ldrElf *elf, WCHAR *arguments);                             /* ElfPack 2.x M*CORE entry point. */
#endif

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 SetLoopTimer(APPLICATION_T *app, UINT32 period);

static UINT32 CheckKeyboard(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 ProcessKeyboard(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 key, BOOL pressed);

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static void FPS_Meter(void);

#if defined(FTR_GFX_ATI)
static UINT32 ATI_Driver_Log(APPLICATION_T *app);
static UINT32 ATI_Driver_Log_Memory(APPLICATION_T *app, AHIPIXFMT_T pixel_format);
static UINT32 ATI_Driver_Start(APPLICATION_T *app);
static UINT32 ATI_Driver_Stop(APPLICATION_T *app);
static UINT32 ATI_Driver_Flush(APPLICATION_T *app);
#elif defined(FTR_GFX_NVIDIA)
static UINT32 Nvidia_Driver_Start(APPLICATION_T *app);
static UINT32 Nvidia_Driver_Stop(APPLICATION_T *app);
static UINT32 Nvidia_Driver_Flush(APPLICATION_T *app);
#elif defined(FTR_GFX_DAL)
static UINT32 DAL_Driver_Start(APPLICATION_T *app);
static UINT32 DAL_Driver_Stop(APPLICATION_T *app);
static UINT32 DAL_Driver_Flush(APPLICATION_T *app);
#endif

static UINT32 GFX_Draw_Start(APPLICATION_T *app);
static UINT32 GFX_Draw_Stop(APPLICATION_T *app);
static UINT32 GFX_Draw_Step(APPLICATION_T *app);

static const char g_app_name[APP_NAME_LEN] = "P2kDoom8";

/*
 * Separate error APP section!!!!
 */

typedef enum {
	APP_ERROR_STATE_ANY,
	APP_ERROR_STATE_INIT,
	APP_ERROR_STATE_MAIN,
	APP_ERROR_STATE_MAX
} APP_ERROR_STATE_T;

typedef enum {
	APP_ERROR_NO,
	APP_ERROR_J2ME_HEAP,
	APP_ERROR_WAD
} APP_ERROR_T;

typedef struct {
	APPLICATION_T app;
} APP_ERROR_INSTANCE_T;

static APP_ERROR_T g_app_error;

#if defined(EP2)
static ldrElf g_app_elf;
#elif defined(EM1)
static ElfLoaderApp g_app_elf = { 0 };
#elif defined(EM2)
static ldrElf *g_app_elf = NULL;
#endif

WCHAR *g_res_file_path_ptr;
static WCHAR g_res_file_path[FS_MAX_URI_NAME_LENGTH];

static const WCHAR *g_msg_title = L"P2kDoom8";
static const WCHAR *g_msg_error_j2me_heap =
	L"Please Run and then Suspend any Java application before running this ELF to initialize the J2ME memory heap!";
static const WCHAR *g_msg_error_wad =  L"Could not find the WAD resource file in the specified path:";

static UINT32 AE_Start(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 AE_Stop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 AE_HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 AE_HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 AE_DeleteDialog(APPLICATION_T *app);

static EVENT_HANDLER_ENTRY_T g_ae_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_ae_state_init_hdls[] = {
	{ EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_ae_state_main_hdls[] = {
	{ EV_DONE, AE_Stop },
	{ EV_DIALOG_DONE, AE_Stop },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_ae_state_table_hdls[] = {
	{ APP_ERROR_STATE_ANY, NULL, NULL, g_ae_state_any_hdls },
	{ APP_ERROR_STATE_INIT, NULL, NULL, g_ae_state_init_hdls },
	{ APP_ERROR_STATE_MAIN, AE_HandleStateEnter, AE_HandleStateExit, g_ae_state_main_hdls }
};

static UINT32 AE_Start(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_ERROR_INSTANCE_T *appi;

	UNUSED(reg_hdl);

	status = RESULT_FAIL;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		appi = (APP_ERROR_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_ERROR_INSTANCE_T),
			reg_id, 0, 1, 1, 1, 1, 0);

		status = APP_Start(ev_st, &appi->app, APP_ERROR_STATE_MAIN,
			g_ae_state_table_hdls, AE_Stop, g_app_name, 0);
	}

	return status;
}

static UINT32 AE_Stop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	APP_ConsumeEv(ev_st, app);

	AE_DeleteDialog(app);

	status = APP_Exit(ev_st, app, 0);

#if defined(EP1)
	LdrUnloadELF(&Lib);
#elif defined(EP2)
	ldrUnloadElf();
#elif defined(EM1)
	LoaderEndApp(&g_app_elf);
#elif defined(EM2)
	ldrUnloadElf(g_app_elf);
#endif

	return status;
}

static UINT32 AE_HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;

	UNUSED(ev_st);

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	AE_DeleteDialog(app);

	port = app->port;

	memclr(&content, sizeof(CONTENT_T));

	switch (g_app_error) {
		case APP_ERROR_WAD:
			UIS_MakeContentFromString("q0Nq1Nq2", &content, g_msg_title, g_msg_error_wad, g_res_file_path_ptr);
			break;
		case APP_ERROR_J2ME_HEAP:
			UIS_MakeContentFromString("q0Nq1", &content, g_msg_title, g_msg_error_j2me_heap);
			break;
		default:
			break;
	}

	dialog = UIS_CreateViewer(&port, &content, NULL);

	if (dialog == DialogType_None) {
		return RESULT_FAIL;
	}

	app->dialog = dialog;

	return RESULT_OK;
}

static UINT32 AE_HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state) {
	UNUSED(ev_st);
	if (state == EXIT_STATE_EXIT) {
		AE_DeleteDialog(app);
		return RESULT_OK;
	}
	return RESULT_FAIL;
}

static UINT32 AE_DeleteDialog(APPLICATION_T *app) {
	if (app->dialog != DialogType_None) {
		UIS_Delete(app->dialog);
		app->dialog = DialogType_None;
		return RESULT_OK;
	}

	return RESULT_FAIL;
}

static APP_ERROR_T CheckEnvironment(void) {
	APP_ERROR_T error = APP_ERROR_NO;

#if !defined(USE_UIS_ALLOCA)
	/* Check if J2ME Heap is present. */
	UINT8 *m_ptr = NULL;
	m_ptr = AmMemAllocPointer(1024);
	if (!m_ptr) {
		LOG("%s\n", "Error: Cannot allocate 1024 byte of J2ME HEAP memory! Is J2ME VM down?");
		error = APP_ERROR_J2ME_HEAP;
	} else {
		AmMemFreePointer(m_ptr);
	}
#endif

	/* Check resource files. */
	*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
	u_strcat(g_res_file_path, L"P2kDoom8.wad");
	g_res_file_path_ptr = g_res_file_path;
	if (!DL_FsFFileExist(g_res_file_path_ptr)) {
		LOG("%s\n", "Error: WAD file is not found!");
		g_res_file_path_ptr = L"/e/mobile/P2kDoom8.wad";
		if (!DL_FsFFileExist(g_res_file_path_ptr)) {
			LOG("%s\n", "Error: WAD file is not found (Second Try)!");
			error = APP_ERROR_WAD;
		}
	}

	g_app_error = error;

	return error;
}

static EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_state_init_hdls[] = {
	{ EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_DONE, ApplicationStop },
	{ EV_DIALOG_DONE, ApplicationStop },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, HandleStateExit, g_state_main_hdls },
};

#if defined(EP1)
UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code) {
	UINT32 status;
	UINT32 ev_code_base;

	UNUSED(args);

	ev_code_base = ev_code;

	u_atou(elf_path_uri, g_res_file_path);

	if (CheckEnvironment() == APP_ERROR_NO) {
		status = APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);
	} else {
		status = APP_Register(&ev_code_base, 1, g_ae_state_table_hdls, APP_ERROR_STATE_MAX, (void *) AE_Start);
	}

	LdrStartApp(ev_code_base);

	return status;
}
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments) {
	UINT32 status;
	UINT32 ev_code_base;
	UINT32 reserve;

	if (ldrIsLoaded(g_app_name)) {
		cprint("P2kDoom8: Error! Application has already been loaded!\n");
		return NULL;
	}

	status = RESULT_OK;

	u_strcpy(g_res_file_path, uri);

	ev_code_base = ldrRequestEventBase();
	reserve = ev_code_base + 1;

	if (CheckEnvironment() == APP_ERROR_NO) {
		reserve = ldrInitEventHandlersTbl(g_state_any_hdls, reserve);
		reserve = ldrInitEventHandlersTbl(g_state_init_hdls, reserve);
		reserve = ldrInitEventHandlersTbl(g_state_main_hdls, reserve);

		status |= APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);
	} else {
		reserve = ldrInitEventHandlersTbl(g_ae_state_any_hdls, reserve);
		reserve = ldrInitEventHandlersTbl(g_ae_state_init_hdls, reserve);
		reserve = ldrInitEventHandlersTbl(g_ae_state_main_hdls, reserve);

		status |= APP_Register(&ev_code_base, 1, g_ae_state_table_hdls, APP_ERROR_STATE_MAX, (void *) AE_Start);
	}
	status |= ldrSendEvent(ev_code_base);
	g_app_elf.name = (char *) g_app_name;

	return (status == RESULT_OK) ? &g_app_elf : NULL;
}
#elif defined(EM1)
int _main(ElfLoaderApp ela) {
	UINT32 status;

	status = RESULT_OK;

	memcpy((void *) &g_app_elf, (void *) &ela, sizeof(ElfLoaderApp));

	u_strcpy(g_res_file_path, uri);

	if (CheckEnvironment() == APP_ERROR_NO) {
		status = APP_Register(&g_app_elf.evcode, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);
	} else {
		status = APP_Register(&g_app_elf.evcode, 1, g_ae_state_table_hdls, APP_ERROR_STATE_MAX, (void *) AE_Start);
	}

	LoaderShowApp(&g_app_elf);

	return RESULT_FAIL;
}
#elif defined(EM2)
UINT32 ELF_Entry(ldrElf *elf, WCHAR *arguments) {
	UINT32 status;
	UINT32 reserve;
	WCHAR *ptr;

	UNUSED(arguments);

	status = RESULT_OK;
	g_app_elf = elf;
	g_app_elf->name = (char *) g_app_name;

	if (ldrIsLoaded(g_app_elf->name)) {
		PFprintf("%s: Application already loaded.\n", g_app_elf->name);
		return RESULT_FAIL;
	}

	ptr = NULL;
	u_strcpy(g_res_file_path, L"file:/");
	ptr = g_res_file_path + u_strlen(g_res_file_path);
	DL_FsGetURIFromID(&g_app_elf->id, ptr);

	reserve = g_app_elf->evbase + 1;
	if (CheckEnvironment() == APP_ERROR_NO) {
		reserve = ldrInitEventHandlersTbl(g_state_any_hdls, reserve);
		reserve = ldrInitEventHandlersTbl(g_state_init_hdls, reserve);
		reserve = ldrInitEventHandlersTbl(g_state_main_hdls, reserve);

		status |= APP_Register(&g_app_elf->evbase, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);
	} else {
		reserve = ldrInitEventHandlersTbl(g_ae_state_any_hdls, reserve);
		reserve = ldrInitEventHandlersTbl(g_ae_state_init_hdls, reserve);
		reserve = ldrInitEventHandlersTbl(g_ae_state_main_hdls, reserve);

		status |= APP_Register(&g_app_elf->evbase, 1, g_ae_state_table_hdls, APP_ERROR_STATE_MAX, (void *) AE_Start);
	}
	if (status == RESULT_OK) {
		PFprintf("%s: Application has been registered successfully.\n", g_app_elf->name);

		status |= ldrSendEvent(g_app_elf->evbase);
	} else {
		PFprintf("%s: Cannot register application.\n", g_app_elf->name);
	}

	return status;
}
#endif

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	UNUSED(reg_hdl);

	status = RESULT_OK;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		appi = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 0, 1, 1, 1, 0);

#if defined(FTR_GFX_ATI)
		appi->ahi.info_driver = NULL;
#elif defined(FTR_GFX_NVIDIA)
		appi->gfsdk.gxHandle = (GF_HANDLE) &GxHandle;
		appi->gfsdk.fb0 = NULL;
#elif defined(FTR_GFX_DAL)
		appi->dal.bitmap = NULL;
#endif
		appi->bmp_width = SCREENWIDTH;
		appi->bmp_height = SCREENHEIGHT;
		appi->p_bitmap = NULL;
		appi->timer_handle = 0;
		appi->keys.pressed = 0;
		appi->keys.released = 0;

		DL_AudGetVolumeSetting(PHONE, &appi->keyboard_volume_level);
		DL_AudSetVolumeSetting(PHONE, 0);

#if defined(FTR_GFX_ATI)
		status |= ATI_Driver_Start((APPLICATION_T *) appi);
#elif defined(FTR_GFX_NVIDIA)
		status |= Nvidia_Driver_Start((APPLICATION_T *) appi);
#elif defined(FTR_GFX_DAL)
		status |= DAL_Driver_Start((APPLICATION_T *) appi);
#endif

		status |= APP_Start(ev_st, &appi->app, APP_STATE_MAIN, g_state_table_hdls, ApplicationStop, g_app_name, 0);

#if defined(EP2)
		g_app_elf.app = (APPLICATION_T *) appi;
#elif defined(EM2)
		g_app_elf->app = &appi;
#endif
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	APP_ConsumeEv(ev_st, app);

	DeleteDialog(app);

	DL_AudSetVolumeSetting(PHONE, appi->keyboard_volume_level);

	status |= GFX_Draw_Stop(app);
	status |= SetLoopTimer(app, 0);
#if defined(FTR_GFX_ATI)
	status |= ATI_Driver_Stop(app);
#elif defined(FTR_GFX_NVIDIA)
	status |= Nvidia_Driver_Stop(app);
#elif defined(FTR_GFX_DAL)
	status |= DAL_Driver_Stop(app);
#endif

	status |= APP_Exit(ev_st, app, 0);

#if defined(EP1)
	LdrUnloadELF(&Lib);
#elif defined(EP2)
	ldrUnloadElf();
#elif defined(EM1)
	LoaderEndApp(&g_app_elf);
#elif defined(EM2)
	ldrUnloadElf(g_app_elf);
#endif
	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	SU_PORT_T port;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;

	UNUSED(ev_st);

	if (state != ENTER_STATE_ENTER) {
		if (app->state != APP_STATE_MAIN) {
			SetLoopTimer(app, TIMER_FAST_UPDATE_MS);
			return RESULT_OK;
		}
	}

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;
	dialog = DialogType_None;

	switch (app_state) {
		case APP_STATE_MAIN:
#if defined(FTR_V600) || defined(FTR_V635)
			{
				DRAWING_BUFFER_T buffer;
				GRAPHIC_POINT_T point;
				point = UIS_CanvasGetDisplaySize();
				buffer.w = point.x + 1;
				buffer.h = point.y + 1;
				buffer.buf = NULL;
				dialog = UIS_CreateColorCanvas(&port, &buffer, TRUE);
			}
#else
			dialog = UIS_CreateNullDialog(&port);
#endif
			DL_KeyKjavaGetKeyState(); /* Reset Keys. */

			if (state == ENTER_STATE_ENTER) {
				GFX_Draw_Start(app);
			}

			SetLoopTimer(app, TIMER_FAST_UPDATE_MS);

			break;
		default:
			break;
	}

	if (dialog == DialogType_None) {
		return RESULT_FAIL;
	}

	app->dialog = dialog;

	return RESULT_OK;
}

static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state) {
	UNUSED(ev_st);

	if (state == EXIT_STATE_EXIT) {
		if (app->state != APP_STATE_MAIN) {
			DeleteDialog(app);
		}
		SetLoopTimer(app, 0);
		return RESULT_OK;
	}

	return RESULT_FAIL;
}

static UINT32 DeleteDialog(APPLICATION_T *app) {
	if (app->dialog != DialogType_None) {
		UIS_Delete(app->dialog);
		app->dialog = DialogType_None;
		return RESULT_OK;
	}
	return RESULT_FAIL;
}

static UINT32 SetLoopTimer(APPLICATION_T *app, UINT32 period) {
	UINT32 status;
	APP_INSTANCE_T *appi;
	IFACE_DATA_T iface_data;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;
	iface_data.port = app->port;

	if (appi->timer_handle != 0) {
		iface_data.handle = appi->timer_handle;
		status |= DL_ClkStopTimer(&iface_data);
	}

	if (period != 0) {
		DL_ClkStartCyclicalTimer(&iface_data, period, APP_TIMER_LOOP);
		status |= appi->timer_handle = iface_data.handle;
	}

	return status;
}

static UINT32 CheckKeyboard(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 key;
	APP_INSTANCE_T *appi;

	key = 0x00080000;

	appi = (APP_INSTANCE_T *) app;
	appi->keys.released = appi->keys.pressed;
	appi->keys.pressed = DL_KeyKjavaGetKeyState();

	while (key) {
		if ((appi->keys.released & key) != (appi->keys.pressed & key)) {
			if (appi->keys.pressed & key) {
				/* Key Pressed. */
				ProcessKeyboard(ev_st, app, key, TRUE);
			}
			if (appi->keys.released & key) {
				/* Key Released. */
				ProcessKeyboard(ev_st, app, key, FALSE);
			}
		}
		key >>= 1;
	}

	return RESULT_OK;
}

static evtype_t modkey_state = ev_keyup;

static UINT32 ProcessKeyboard(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 key, BOOL pressed) {
#if defined(KEYS_PORTRAIT)
	#define KK_2 MULTIKEY_2
	#define KK_UP MULTIKEY_UP
	#define KK_4 MULTIKEY_4
	#define KK_LEFT MULTIKEY_LEFT
	#define KK_6 MULTIKEY_6
	#define KK_RIGHT MULTIKEY_RIGHT
	#define KK_8 MULTIKEY_8
	#define KK_DOWN MULTIKEY_DOWN
#else
	#define KK_2 MULTIKEY_6
	#define KK_UP MULTIKEY_RIGHT
	#define KK_4 MULTIKEY_2
	#define KK_LEFT MULTIKEY_UP
	#define KK_6 MULTIKEY_8
	#define KK_RIGHT MULTIKEY_DOWN
	#define KK_8 MULTIKEY_4
	#define KK_DOWN MULTIKEY_LEFT
#endif

	UNUSED(ev_st);

	event_t ev;
	ev.data1 = 0;
	ev.type = (pressed) ? ev_keydown : ev_keyup;

	switch (key) {
		case MULTIKEY_0:
		case MULTIKEY_SOFT_LEFT:
			app->exit_status = TRUE;
			break;
		case MULTIKEY_1:
			if (modkey_state == ev_keyup) {
				ev.data1 = KEYD_START;
			} else {
				if (ev.type == ev_keydown) {
					Apply_Cheat(CHEAT_IDRATE_FPS);
				}
			}
			break;
		case KK_2:
		case KK_UP:
			if (modkey_state == ev_keyup) {
				ev.data1 = KEYD_UP;
			} else {
				if (ev.type == ev_keydown) {
					Apply_Cheat(CHEAT_LEVEL_END);
				}
			}
			break;
		case MULTIKEY_3:
			modkey_state = ev.type;
			ev.data1 = KEYD_STRAFE;
			break;
		case KK_4:
		case KK_LEFT:
			ev.data1 = KEYD_LEFT;
			break;
		case MULTIKEY_5:
		case MULTIKEY_JOY_OK:
			if (modkey_state == ev_keyup) {
				ev.data1 = KEYD_B;
			} else {
				if (ev.type == ev_keydown) {
					Apply_Cheat(CHEAT_IDDQD_GOD);
				}
			}
			break;
		case KK_6:
		case KK_RIGHT:
			ev.data1 = KEYD_RIGHT;
			break;
		case MULTIKEY_7:
			if (modkey_state == ev_keyup) {
				ev.data1 = KEYD_A;
			} else {
				if (ev.type == ev_keydown) {
					Apply_Cheat(CHEAT_IDKFA_GIVE_ALL);
				}
			}
			break;
		case MULTIKEY_9:
			if (modkey_state == ev_keyup) {
				ev.data1 = KEYD_SELECT;
			} else {
				if (ev.type == ev_keydown) {
					Apply_Cheat(CHEAT_CHOPPERS_CHAINSAW);
				}
			}
			break;
		case KK_8:
		case KK_DOWN:
			ev.data1 = KEYD_DOWN;
			break;
		case MULTIKEY_STAR:
			ev.data1 = KEYD_BRACKET_LEFT;
			break;
		case MULTIKEY_POUND:
			ev.data1 = KEYD_BRACKET_RIGHT;
			break;
		case MULTIKEY_SOFT_RIGHT:
			if (modkey_state == ev_keyup) {
				ev.data1 = KEYD_A;
			} else {
				if (ev.type == ev_keydown) {
					Apply_Cheat(CHEAT_ROCKETS_ENABLE);
				}
			}
			break;
		default:
			break;
	}

	if(ev.data1 != 0)
		D_PostEvent(&ev);

	return RESULT_OK;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	APP_ConsumeEv(ev_st, app);

	switch (timer_id) {
		case APP_TIMER_LOOP:
			FPS_Meter();
			CheckKeyboard(ev_st, app);
			GFX_Draw_Step(app);
#if defined(FTR_GFX_ATI)
			ATI_Driver_Flush(app);
#elif defined(FTR_GFX_NVIDIA)
			Nvidia_Driver_Flush(app);
#elif defined(FTR_GFX_DAL)
			DAL_Driver_Flush(app);
#endif
			break;
		case APP_TIMER_EXIT:
			/* Play an exit sound using quiet speaker. */
			DL_AudPlayTone(0x00,  0xFF);
			return ApplicationStop(ev_st, app);
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static void FPS_Meter(void) {
#if defined(FPS_METER)
	UINT64 current_time;
	UINT32 delta;

	static UINT32 one = 0;
	static UINT64 last_time = 0;
	static UINT32 tick = 0;
	static UINT32 fps = 0;

	current_time = suPalTicksToMsec(suPalReadTime());
	delta = (UINT32) (current_time - last_time);
	last_time = current_time;

	tick = (tick + delta) / 2;
	if (tick != 0) {
		fps = 1000 * 10 / tick;
	}

	if (one > 30) {
		UtilLogStringData("FPS: %d.%d\n", fps / 10, fps % 10);
		PFprintf("FPS: %d.%d\n", fps / 10, fps % 10);
#if defined(EP2)
		cprintf("FPS: %d.%d\n", fps / 10, fps % 10);
#endif
		one = 0;
	}
	one++;
#endif
}

#if defined(FULLSCREEN_240X320) || defined(FULLSCREEN_320X240)
#define VIDEO_W 240
#define VIDEO_H 320
#elif defined(FULLSCREEN_176X220) || defined(FULLSCREEN_220X176)
#define VIDEO_W 176
#define VIDEO_H 220
#elif defined(FTR_C650)
#define VIDEO_W 128
#define VIDEO_H 128
#elif defined(FTR_L6)
#define VIDEO_W 128
#define VIDEO_H 160
#else
#define VIDEO_W 240
#define VIDEO_H 320
#endif

static UINT32 doom_current_palette[256];

#if defined(FTR_GFX_ATI)
static UINT32 ATI_Driver_Log(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	LOG("%s\n", "ATI Driver Dump.");
	LOG("ATI Driver Name: %s\n", appi->ahi.info_driver->drvName);
	LOG("ATI Driver Version: %s\n", appi->ahi.info_driver->drvVer);
	LOG("ATI S/W Revision: %d (0x%08X)\n",
		appi->ahi.info_driver->swRevision, appi->ahi.info_driver->swRevision);
	LOG("ATI Chip ID: %d (0x%08X)\n",
		appi->ahi.info_driver->chipId, appi->ahi.info_driver->chipId);
	LOG("ATI Revision ID: %d (0x%08X)\n",
		appi->ahi.info_driver->revisionId, appi->ahi.info_driver->revisionId);
	LOG("ATI CPU Bus Interface Mode: %d (0x%08X)\n",
		appi->ahi.info_driver->cpuBusInterfaceMode, appi->ahi.info_driver->cpuBusInterfaceMode);
	LOG("ATI Total Memory: %d (%d KiB)\n",
		appi->ahi.info_driver->totalMemory, appi->ahi.info_driver->totalMemory / 1024);
	LOG("ATI Internal Memory: %d (%d KiB)\n",
		appi->ahi.info_driver->internalMemSize, appi->ahi.info_driver->internalMemSize / 1024);
	LOG("ATI External Memory: %d (%d KiB)\n",
		appi->ahi.info_driver->externalMemSize, appi->ahi.info_driver->externalMemSize / 1024);
	LOG("ATI CAPS 1: %d (0x%08X)\n", appi->ahi.info_driver->caps1, appi->ahi.info_driver->caps1);
	LOG("ATI CAPS 2: %d (0x%08X)\n", appi->ahi.info_driver->caps2, appi->ahi.info_driver->caps2);

	LOG("ATI Surface Screen Info: width=%d, height=%d, pixFormat=%d, byteSize=%d, byteSize=%d KiB\n",
		appi->ahi.info_surface_screen.width, appi->ahi.info_surface_screen.height,
		appi->ahi.info_surface_screen.pixFormat,
		appi->ahi.info_surface_screen.byteSize, appi->ahi.info_surface_screen.byteSize / 1024);
	LOG("ATI Surface Screen Info: offset=%d, stride=%d, numPlanes=%d\n",
		appi->ahi.info_surface_screen.offset,
		appi->ahi.info_surface_screen.stride,
		appi->ahi.info_surface_screen.numPlanes);

	LOG("ATI Surface Draw Info: width=%d, height=%d, pixFormat=%d, byteSize=%d, byteSize=%d KiB\n",
		appi->ahi.info_surface_draw.width, appi->ahi.info_surface_draw.height,
		appi->ahi.info_surface_draw.pixFormat,
		appi->ahi.info_surface_draw.byteSize, appi->ahi.info_surface_draw.byteSize / 1024);
	LOG("ATI Surface Draw Info: offset=%d, stride=%d, numPlanes=%d\n",
		appi->ahi.info_surface_draw.offset,
		appi->ahi.info_surface_draw.stride,
		appi->ahi.info_surface_draw.numPlanes);

	return RESULT_OK;
}

static UINT32 ATI_Driver_Log_Memory(APPLICATION_T *app, AHIPIXFMT_T pixel_format) {
	enum {
		INTERNAL_MEMORY,
		EXTERNAL_MEMORY,
		SYSTEM_MEMORY,
		MEMORY_MAX
	};

	APP_INSTANCE_T *appi;
	UINT32 status[MEMORY_MAX] = { 0 };
	UINT32 sizes[MEMORY_MAX] = { 0 };
	UINT32 alignment[MEMORY_MAX] = { 0 };

	appi = (APP_INSTANCE_T *) app;

	status[INTERNAL_MEMORY] = AhiSurfGetLargestFreeBlockSize(appi->ahi.context, pixel_format,
		&sizes[INTERNAL_MEMORY], &alignment[INTERNAL_MEMORY], AHIFLAG_INTMEMORY);
	status[EXTERNAL_MEMORY] = AhiSurfGetLargestFreeBlockSize(appi->ahi.context, pixel_format,
		&sizes[EXTERNAL_MEMORY], &alignment[EXTERNAL_MEMORY], AHIFLAG_EXTMEMORY);
	status[SYSTEM_MEMORY] = AhiSurfGetLargestFreeBlockSize(appi->ahi.context, pixel_format,
		&sizes[SYSTEM_MEMORY], &alignment[SYSTEM_MEMORY], AHIFLAG_SYSMEMORY);

	LOG("%s\n", "ATI Memory Dump.");
	LOG("\tATI Internal Memory Largest Block: status=%d, pixel_format=%d, size=%d, size=%d KiB, align=%d\n",
		status[INTERNAL_MEMORY], pixel_format, sizes[INTERNAL_MEMORY], sizes[INTERNAL_MEMORY] / 1024, alignment[INTERNAL_MEMORY]);
	LOG("\tATI External Memory Largest Block: status=%d, pixel_format=%d, size=%d, size=%d KiB, align=%d\n",
		status[EXTERNAL_MEMORY], pixel_format, sizes[EXTERNAL_MEMORY], sizes[EXTERNAL_MEMORY] / 1024, alignment[EXTERNAL_MEMORY]);
	LOG("\tATI System Memory Largest Block: status=%d, pixel_format=%d, size=%d, size=%d KiB, align=%d\n",
		status[SYSTEM_MEMORY], pixel_format, sizes[SYSTEM_MEMORY], sizes[SYSTEM_MEMORY] / 1024, alignment[SYSTEM_MEMORY]);

	return status[INTERNAL_MEMORY] && status[EXTERNAL_MEMORY] && status[SYSTEM_MEMORY];
}

#if defined(SET_DISPLAY_MODE)
static UINT32 Find_Surface_Addresses_In_RAM(APPLICATION_T *app, UINT32 start_address, UINT32 size_search_region) {
	APP_INSTANCE_T *appi;
	UINT32 surface_block_offset;
	UINT32 *start_ptr;

	appi = (APP_INSTANCE_T *) app;
	surface_block_offset = 0;
	start_ptr = (UINT32 *) start_address;

	while (surface_block_offset < size_search_region) {
		/*
		 * Find the block of:
		 * o-4: ...
		 * o  : curDrawSurf
		 * o+4: curDispSurf
		 * o+8: ...
		*/

		if(*start_ptr == (UINT32) (appi->ahi.screen)) {
			if(*(start_ptr + 1) == (UINT32) (appi->ahi.draw)) {
				break;
			}
		}

		start_ptr++;
		surface_block_offset += 4;
	}
	if (surface_block_offset >= size_search_region) {
		/* Not Found? */
		surface_block_offset = 0;
	}

	return surface_block_offset;
}

static UINT32 ATI_Driver_Set_Display_Mode(APPLICATION_T *app, AHIROTATE_T mode) {
	UINT32 status;
	APP_INSTANCE_T *appi;
	AHIPOINT_T size_landscape;
	AHIPOINT_T size_landscape_240x160;
	AHIPOINT_T size_portrait;
	AHIDEVCONTEXT_T device_context;
	UINT32 *surface_disp_addr;
	UINT32 *surface_draw_addr;
	AHIDISPMODE_T display_mode;
	UINT32 start_addr;
	UINT32 search_region;
	UINT32 surface_block_offset;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	device_context = DAL_GetDeviceContext(DISPLAY_MAIN);

	/* Use this if `Class_dal` constant is unknown or buggy. */
//	start_addr = 0x12000000;
//	search_region = 0x03FFFFFF; /* 4 MB RAM */
//	search_region = 0x07FFFFFF; /* 8 MB RAM */
	start_addr = (UINT32) &Class_dal;
#if !defined(SEARCH_LONG_RANGE)
	search_region = 0x00000100;
#else
	search_region = 0x00100000; /* Damn! This can be 0x100000 for Motorola RAZR V3r! */
#endif
	surface_block_offset = Find_Surface_Addresses_In_RAM(app, start_addr, search_region);

	status |= AhiDispModeGet(appi->ahi.context, &display_mode);

	size_landscape.x = display_mode.size.y;
	size_landscape.y = display_mode.size.x;
	size_landscape_240x160.x = SCREENWIDTH;
	size_landscape_240x160.y = SCREENHEIGHT;
	size_portrait.x = display_mode.size.x;
	size_portrait.y = display_mode.size.y;

	surface_disp_addr = (UINT32 *) (start_addr + surface_block_offset + 0x00);
	surface_draw_addr = (UINT32 *) (start_addr + surface_block_offset + 0x04);

	LOG("ATI Display Mode Dumps 1:\n\t"
		"Class_dal=0x%08X 0x%08X 0x%08X\n\t"
		"start_addr=0x%08X 0x%08X 0x%08X\n",
		*Class_dal, &Class_dal, Class_dal, *((UINT32 *)start_addr), &start_addr, start_addr);

	LOG("ATI Display Mode Dumps 2:\n\t"
		"search_region=0x%08X\n\t"
		"surface_disp_addr=0x%08X\n\t"
		"surface_draw_addr=0x%08X\n\t"
		"surface_block_offset=0x%08X\n",
		search_region, surface_disp_addr, surface_draw_addr, surface_block_offset);

	LOG("ATI Display Mode Dumps 3:\n\t"
		"display_source_buffer=0x%08X 0x%08X\n\t"
		"appi->ahi.screen=0x%08X 0x%08X\n\t"
		"appi->ahi.draw=0x%08X 0x%08X\n",
		display_source_buffer, &display_source_buffer,
		(UINT32) (appi->ahi.screen), &appi->ahi.screen,
		(UINT32) (appi->ahi.draw), &appi->ahi.draw);

	if (mode == AHIROT_90 || mode == AHIROT_270) {
		status |= AhiSurfFree(device_context, (AHISURFACE_T) (*surface_disp_addr));
		status |= AhiSurfFree(device_context, (AHISURFACE_T) (*surface_draw_addr));

		status |= ATI_Driver_Log_Memory(app, AHIFMT_8BPP);

		status = AhiSurfAlloc(appi->ahi.context,
			&appi->ahi.screen, &size_landscape, AHIFMT_16BPP_565, AHIFLAG_INTMEMORY);
		if (status != RESULT_OK) {
			LOG("ATI_Driver_Set_Display_Mode: Cannot allocate display (landscape) surface, status = %d\n", status);
			return RESULT_FAIL;
		}
		status = AhiSurfAlloc(appi->ahi.context,
			&appi->ahi.draw, &size_landscape_240x160, AHIFMT_16BPP_565, AHIFLAG_INTMEMORY);
		if (status != RESULT_OK) {
			LOG("ATI_Driver_Set_Display_Mode: Cannot allocate drawing (landscape) surface, status = %d\n", status);
			status |= AhiSurfFree(device_context, appi->ahi.screen);
			appi->ahi.screen = 0;
			return RESULT_FAIL;
		}

		status |= ATI_Driver_Log_Memory(app, AHIFMT_8BPP);
	} else {
		if (appi->ahi.screen) {
			status |= AhiSurfFree(appi->ahi.context, appi->ahi.screen);
		}
		if (appi->ahi.draw) {
			status |= AhiSurfFree(appi->ahi.context, appi->ahi.draw);
		}

		status |= ATI_Driver_Log_Memory(app, AHIFMT_8BPP);

		status = AhiSurfAlloc(device_context,
			&appi->ahi.screen, &size_portrait, AHIFMT_16BPP_565, AHIFLAG_INTMEMORY);
		if (status != RESULT_OK) {
			LOG("ATI_Driver_Set_Display_Mode: Cannot allocate display (portrait) surface, status = %d\n", status);
			return RESULT_FAIL;
		}
		status = AhiSurfAlloc(device_context,
			&appi->ahi.draw, &size_portrait, AHIFMT_16BPP_565, 0);
		if (status != RESULT_OK) {
			LOG("ATI_Driver_Set_Display_Mode: Cannot allocate drawing (portrait) surface, status = %d\n", status);
			status |= AhiSurfFree(device_context, appi->ahi.screen);
			appi->ahi.screen = 0;
			return RESULT_FAIL;
		}

		*surface_disp_addr = (UINT32) appi->ahi.screen;
		*surface_draw_addr = (UINT32) appi->ahi.draw;

		status |= ATI_Driver_Log_Memory(app, AHIFMT_8BPP);
	}

	status |= AhiDispModeGet(appi->ahi.context, &display_mode);

#if defined(FTR_V600) || defined(FTR_V635)
	DAL_DisableDisplay(DISPLAY_MAIN);
#else
	status |= AhiDispState(appi->ahi.context, DISPLAY_OFF, 0);
#endif

	display_mode.rotation = mode;
	status = AhiDispModeSet(appi->ahi.context, &display_mode, 0);
	if (status != RESULT_OK) {
		LOG("ATI_Driver_Set_Display_Mode: Cannot change display mode, status = %d\n", status);
		return RESULT_FAIL;
	}
	status |= AhiDispSurfSet(appi->ahi.context, appi->ahi.screen, 0);

#if defined(FTR_V600) || defined(FTR_V635)
	DAL_EnableDisplay(DISPLAY_MAIN);
#else
	status |= AhiDispState(appi->ahi.context, DISPLAY_ON, 0);
#endif

	return status;
}
#endif /* defined(SET_DISPLAY_MODE) */

static UINT32 ATI_Driver_Start(APPLICATION_T *app) {
	UINT32 status;
	INT32 result;
	APP_INSTANCE_T *appi;
	AHIDEVICE_T ahi_device;
	AHIDISPMODE_T display_mode;

	status = RESULT_OK;
	result = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	LOG("%s\n", "ATI Driver Start Initialization.");

	appi->ahi.info_driver = suAllocMem(sizeof(AHIDRVINFO_T), &result);
	if (!appi->ahi.info_driver && result) {
		return RESULT_FAIL;
	}
	status |= AhiDevEnum(&ahi_device, appi->ahi.info_driver, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}
	status |= AhiDevOpen(&appi->ahi.context, ahi_device, g_app_name, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}

	status |= AhiDispModeGet(appi->ahi.context, &display_mode);

	status |= AhiDispSurfGet(appi->ahi.context, &appi->ahi.screen);
	appi->ahi.draw = DAL_GetDrawingSurface(DISPLAY_MAIN);

	/*
	 * Motorola SLVR L6: 128x160
	 * Motorola ROKR E1: 176x220
	 */
	appi->is_CSTN_display =
			(display_mode.size.x < 176) ||
			(display_mode.size.y < 220);

#if defined(SET_DISPLAY_MODE)
#if defined(FTR_L6)
	status |= ATI_Driver_Set_Display_Mode(app, AHIROT_90);
#else
	status |= ATI_Driver_Set_Display_Mode(app, (appi->is_CSTN_display) ? AHIROT_270 : AHIROT_90);
#endif
#endif

	status |= AhiDrawClipDstSet(appi->ahi.context, NULL);
	status |= AhiDrawClipSrcSet(appi->ahi.context, NULL);

	status |= AhiSurfInfo(appi->ahi.context, appi->ahi.screen, &appi->ahi.info_surface_screen);
	status |= AhiSurfInfo(appi->ahi.context, appi->ahi.draw, &appi->ahi.info_surface_draw);

	appi->width = appi->ahi.info_surface_screen.width;
	appi->height = appi->ahi.info_surface_screen.height;

	appi->ahi.update_params.size = sizeof(AHIUPDATEPARAMS_T);
	appi->ahi.update_params.sync = FALSE;
	appi->ahi.update_params.rect.x1 = 0;
	appi->ahi.update_params.rect.y1 = 0;
#if defined(SET_DISPLAY_MODE)
	appi->ahi.update_params.rect.x2 = 0 + display_mode.size.y;
	appi->ahi.update_params.rect.y2 = 0 + display_mode.size.x;
#else
	appi->ahi.update_params.rect.x2 = 0 + display_mode.size.x;
	appi->ahi.update_params.rect.y2 = 0 + display_mode.size.y;
#endif
	appi->ahi.point_bitmap.x = 0;
	appi->ahi.point_bitmap.y = 0;

	appi->ahi.bitmap.width = appi->bmp_width;
	appi->ahi.bitmap.height = appi->bmp_height;
	appi->ahi.bitmap.stride = appi->bmp_width; /* (width * bpp) */
	appi->ahi.bitmap.format = AHIFMT_8BPP;

	appi->ahi.bitmap.image = suAllocMem(SCREENWIDTH * SCREENHEIGHT, &result);
	if (result != RESULT_OK) {
		LOG("%s\n", "Error: Cannot allocate screen buffer bitmap image.");
	}

	appi->ahi.rect_bitmap.x1 = 0;
	appi->ahi.rect_bitmap.y1 = 0;
	appi->ahi.rect_bitmap.x2 = 0 + appi->bmp_width;
	appi->ahi.rect_bitmap.y2 = 0 + appi->bmp_height;

#if defined(ROT_90)
	appi->ahi.rect_draw.x1 = 0;
	appi->ahi.rect_draw.y1 = appi->bmp_height + 1;
	appi->ahi.rect_draw.x2 = 0 + appi->bmp_height;
	appi->ahi.rect_draw.y2 = appi->bmp_height + 1 + appi->bmp_width;
#elif defined(ROT_0)
	appi->ahi.rect_draw.x1 = appi->width / 2 - appi->bmp_width / 2;
	appi->ahi.rect_draw.y1 = appi->height / 2 - appi->bmp_height / 2;
	appi->ahi.rect_draw.x2 = (appi->width / 2 - appi->bmp_width / 2) + appi->bmp_width;
	appi->ahi.rect_draw.y2 = (appi->height / 2 - appi->bmp_height / 2) + appi->bmp_height;

	status |= AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.screen, 0);

	status |= AhiDrawBrushFgColorSet(appi->ahi.context, ATI_565RGB(0x88, 0x88, 0x88));
	status |= AhiDrawBrushSet(appi->ahi.context, NULL, NULL, 0, AHIFLAG_BRUSH_SOLID);
	status |= AhiDrawRopSet(appi->ahi.context, AHIROP3(AHIROP_PATCOPY));
	status |= AhiDrawSpans(appi->ahi.context, &appi->ahi.update_params.rect, 1, 0);
#endif

	AhiDrawRopSet(appi->ahi.context, AHIROP3(AHIROP_SRCCOPY));

	status |= ATI_Driver_Log(app);
	status |= ATI_Driver_Log_Memory(app, AHIFMT_16BPP_565);

	return status;
}

static UINT32 ATI_Driver_Stop(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

#if defined(SET_DISPLAY_MODE)
#if defined(FTR_L6)
	status |= ATI_Driver_Set_Display_Mode(app, AHIROT_0);
#else
	status |= ATI_Driver_Set_Display_Mode(app, (appi->is_CSTN_display) ? AHIROT_180 : AHIROT_0);
#endif
#endif

	if (appi->ahi.bitmap.image) {
		LOG("%s\n", "Free: ATI Bitmap memory.");
		suFreeMem(appi->ahi.bitmap.image);
		appi->ahi.bitmap.image = NULL;
		appi->p_bitmap = NULL;
	}

	status |= AhiDevClose(appi->ahi.context);
	if (appi->ahi.info_driver) {
		LOG("%s\n", "Free: ATI Driver Info memory.");
		suFreeMem(appi->ahi.info_driver);
	}

	return status;
}

static UINT32 ATI_Driver_Flush(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

#if defined(SET_DISPLAY_MODE)

	AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.draw, 0);
	AhiDrawBitmapBlt(appi->ahi.context,
		&appi->ahi.rect_bitmap, &appi->ahi.point_bitmap, &appi->ahi.bitmap, (void *) doom_current_palette, 0);

	AhiDrawSurfSrcSet(appi->ahi.context, appi->ahi.draw, 0);
	AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.screen, 0);

	AhiDrawStretchBlt(appi->ahi.context, &appi->ahi.update_params.rect, &appi->ahi.rect_bitmap, AHIFLAG_STRETCHFAST);

#else

#if defined(ROT_0)
	AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.screen, 0);
	AhiDrawBitmapBlt(appi->ahi.context,
		&appi->ahi.rect_draw, &appi->ahi.point_bitmap, &appi->ahi.bitmap, (void *) doom_current_palette, 0);
	AhiDispWaitVBlank(appi->ahi.context, 0);
#elif defined(ROT_90)
	AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.draw, 0);
	AhiDrawBitmapBlt(appi->ahi.context,
		&appi->ahi.rect_bitmap, &appi->ahi.point_bitmap, &appi->ahi.bitmap, (void *) doom_current_palette, 0);

	AhiDrawRotateBlt(appi->ahi.context,
		&appi->ahi.rect_draw, &appi->ahi.point_bitmap, AHIROT_90, AHIMIRR_NO, 0);

	AhiDrawSurfSrcSet(appi->ahi.context, appi->ahi.draw, 0);
	AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.screen, 0);

	AhiDispWaitVBlank(appi->ahi.context, 0);
	AhiDrawStretchBlt(appi->ahi.context, &appi->ahi.update_params.rect, &appi->ahi.rect_draw, AHIFLAG_STRETCHFAST);
#endif

#endif // defined(SET_DISPLAY_MODE)
	if (appi->is_CSTN_display) {
		AhiDispUpdate(appi->ahi.context, &appi->ahi.update_params);
	}

	return RESULT_OK;
}
#elif defined(FTR_GFX_DAL)
/* Pure DAL driver for C650-like phones. */
static UINT32 DAL_Driver_Start(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;
	UINT8 *display_bitmap;
	INT32 status;

	appi = (APP_INSTANCE_T *) app;
	appi->dal.draw_region.ulc.x = 0;
	appi->dal.draw_region.ulc.y = 0;
	appi->dal.draw_region.lrc.x = VIDEO_W - 1;
	appi->dal.draw_region.lrc.y = VIDEO_H - 1;


	appi->dal.bitmap = uisAllocateMemory(VIDEO_W * VIDEO_H * 2, &status);
	if (status != RESULT_OK) {
		LOG("%s\n", "Error: Cannot allocate screen buffer bitmap image.");
	}

	return status;
}

static UINT32 DAL_Driver_Stop(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	uisFreeMemory(appi->dal.bitmap);

	return RESULT_OK;
}

static UINT32 DAL_Driver_Flush(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

//	DAL_UpdateDisplayRegion(&appi->dal.draw_region, display_bitmap);
	//DAL_UpdateRectangleDisplayRegion(&appi->dal.draw_region, (UINT16 *) appi->p_bitmap, DISPLAY_MAIN);
	DAL_WriteDisplayRegion(&appi->dal.draw_region, (UINT16 *) appi->p_bitmap, DISPLAY_MAIN, FALSE);

	return RESULT_OK;
}
#elif defined(FTR_GFX_NVIDIA)
static UINT32 Nvidia_Driver_Start(APPLICATION_T *app) {
	INT32 result;
	UINT32 status;
	APP_INSTANCE_T *appi;
	GRAPHIC_POINT_T point;

	result = RESULT_OK;
	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	appi->gfsdk.fb0_rect.x = 0;
	appi->gfsdk.fb0_rect.y = 0;
#if defined(EA1)
	point = UIS_CanvasGetDisplaySize();
#else
	UIS_CanvasGetDisplaySize(&point);
#endif
	appi->gfsdk.fb0_rect.w = point.x + 1;
	appi->gfsdk.fb0_rect.h = point.y + 1;
	appi->gfsdk.fb0 = uisAllocateMemory(appi->gfsdk.fb0_rect.w * appi->gfsdk.fb0_rect.h * 2, &result);
	if (result != RESULT_OK) {
		LOG("Cannot allocate '%d' bytes of memory for fill screen!\n",
			appi->gfsdk.fb0_rect.w * appi->gfsdk.fb0_rect.h * 2);
		status = RESULT_FAIL;
	}
	memclr(appi->gfsdk.fb0, appi->gfsdk.fb0_rect.w * appi->gfsdk.fb0_rect.h * 2);
	Nvidia_Driver_Flush(app);
	uisFreeMemory(appi->gfsdk.fb0);

	appi->gfsdk.fb0_rect.w = point.x + 1;
#if defined(NVIDIA_FULLSCREEN)
	appi->gfsdk.fb0_rect.h = point.y + 1;
#else
	appi->gfsdk.fb0_rect.h = SCREENHEIGHT;
#endif

	// EXL, 28-Jul-2025: Uncomment it to emulate Motorola E770v display on Motorola RAZR V3x screen.
//	appi->gfsdk.fb0_rect.x = 0;
//	appi->gfsdk.fb0_rect.y = 0;
//	appi->gfsdk.fb0_rect.w = 176;
//	appi->gfsdk.fb0_rect.h = 220;

	appi->gfsdk.fb0 = uisAllocateMemory(appi->gfsdk.fb0_rect.w * appi->gfsdk.fb0_rect.h * 2, &result);
	if (result != RESULT_OK) {
		LOG("Cannot allocate '%d' bytes of memory for screen!\n", appi->gfsdk.fb0_rect.w * appi->gfsdk.fb0_rect.h * 2);
		status = RESULT_FAIL;
	}

#if !defined(NVIDIA_FULLSCREEN)
	appi->gfsdk.fb0_rect.y = (point.y + 1) / 2 - SCREENHEIGHT / 2;
#endif

	return status;
}

static UINT32 Nvidia_Driver_Stop(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	uisFreeMemory(appi->gfsdk.fb0);

	return status;
}

static UINT32 Nvidia_Driver_Flush(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	GFGxCopyColorBitmap(
		appi->gfsdk.gxHandle,
		appi->gfsdk.fb0_rect.x, appi->gfsdk.fb0_rect.y,
		appi->gfsdk.fb0_rect.w, appi->gfsdk.fb0_rect.h,
#if defined(NVIDIA_FULLSCREEN)
		appi->gfsdk.fb0_rect.x, appi->gfsdk.fb0_rect.y,
#else
		appi->gfsdk.fb0_rect.x, 0,
#endif
		appi->gfsdk.fb0_rect.w * 2,
		appi->gfsdk.fb0
	);

	return status;
}
#endif

uint8_t *dosAllocatedMem;
uint8_t *doomXMSHandle;

extern const int16_t CENTERY;

// The screen is [SCREENWIDTH * SCREENHEIGHT];
static uint8_t __far* _s_screen;

static int16_t palettelumpnum;

static UINT16 *pp_bitmap;
static APP_INSTANCE_T *appi_g;

static UINT32 GFX_Draw_Start(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

#if defined(FTR_GFX_ATI)
	appi->p_bitmap = (UINT8 *) appi->ahi.bitmap.image;
#elif defined(FTR_GFX_NVIDIA)
	appi->p_bitmap = (UINT8 *) appi->gfsdk.fb0;
#elif defined(FTR_GFX_DAL)
	appi->p_bitmap = (UINT8 *) appi->dal.bitmap;
#endif

	pp_bitmap = (UINT16 *) appi->p_bitmap;
	appi_g = appi;

	dosAllocatedMem = NULL;
	doomXMSHandle = NULL;

	init_main(0, NULL);

	return RESULT_OK;
}

static UINT32 GFX_Draw_Stop(APPLICATION_T *app) {
	UNUSED(app);
#if defined(FTR_GFX_NVIDIA) || defined(FTR_GFX_DAL)
	uisFreeMemory(_s_screen);
#endif

	if (dosAllocatedMem) {
		LOG("%s\n", "Freed: dosAllocatedMem!");
#if defined(USE_UIS_ALLOCA)
		uisFreeMemory(dosAllocatedMem);
#else
		AmMemFreePointer(dosAllocatedMem);
#endif
	}

	if (doomXMSHandle) {
		LOG("%s\n", "Freed: doomXMSHandle!");
#if defined(USE_UIS_ALLOCA)
		uisFreeMemory(doomXMSHandle);
#else
		AmMemFreePointer(doomXMSHandle);
#endif
	}

	return RESULT_OK;
}

static UINT32 GFX_Draw_Step(APPLICATION_T *app) {
	UNUSED(app);

	D_DoomStep();

	return RESULT_OK;
}

void I_ReloadPalette(void)
{
	char lumpName[9] = "PLAYPAL0";

	if (_g_gamma == 0)
		lumpName[7] = 0;
	else
		lumpName[7] = '0' + _g_gamma;

	palettelumpnum = W_GetNumForName(lumpName);
}

#define PEL_WRITE_ADR   0x3c8
#define PEL_DATA        0x3c9

#if 0
static const uint8_t colors[14][3] =
{
	// normal
	{0, 0, 0},

	// red
	{0x07, 0, 0},
	{0x0e, 0, 0},
	{0x15, 0, 0},
	{0x1c, 0, 0},
	{0x23, 0, 0},
	{0x2a, 0, 0},
	{0x31, 0, 0},
	{0x3b, 0, 0},

	// yellow
	{0x06, 0x05, 0x02},
	{0x0d, 0x0b, 0x04},
	{0x14, 0x11, 0x06},
	{0x1a, 0x17, 0x08},

	// green
	{0, 0x08, 0}
};
#endif

#if defined(FTR_GFX_NVIDIA) || defined(FTR_GFX_DAL)
#define ATI_565RGB(r, g, b) (UINT32)((r & 0xf8) << 8 | (g & 0xFC) << 3 | (b & 0xf8) >> 3)
#endif

static void I_UploadNewPalette(int8_t pal)
{
	// This is used to replace the current 256 colour cmap with a new one
	// Used by 256 colour PseudoColor modes

	const uint8_t __far* palette_lump = W_TryGetLumpByNum(palettelumpnum);
	if (palette_lump != NULL)
	{
		const byte __far* palette = &palette_lump[pal * 256 * 3];
#if 0
		outp(PEL_WRITE_ADR, 0);
		for (int_fast16_t i = 0; i < 256 * 3; i++)
			outp(PEL_DATA, (*palette++) >> 2);
#endif

		for(int p = 0; p < 256; p++) {
			doom_current_palette[p] = ATI_565RGB(palette[3*p], palette[(3*p)+1], palette[(3*p)+2]);
		}

#if 0
		outp(PEL_WRITE_ADR, 0);
		for (int_fast16_t i = 0; i < 256 * 3; i++)
			outp(PEL_DATA, (*palette++) >> 2);
#endif

		Z_ChangeTagToCache(palette_lump);
	}
	else
	{
#if 0
		outp(PEL_WRITE_ADR, 0);
		outp(PEL_DATA, colors[pal][0]);
		outp(PEL_DATA, colors[pal][1]);
		outp(PEL_DATA, colors[pal][2]);
#endif
	}
}

#if defined(FTR_GFX_NVIDIA) || defined(FTR_GFX_DAL)
static uint16_t *xtable = NULL;
static uint16_t *ytable = NULL;
static uint16_t *indextable = NULL;

// Generated by ChatGPT-4.1 by GitHub Copilot: https://github.com/copilot/
static void genindextable(int video_w, int video_h) {
	INT32 result;
	indextable = (uint16_t*) uisAllocateMemory(sizeof(uint16_t) * video_w * video_h, &result);
	if (result != RESULT_OK) {
		LOG("%s\n", "Cannot allocate: indextable!");
	}
	for (int y = 0; y < video_h; ++y) {
		int src_y = (y * SCREENHEIGHT) / video_h;
		for (int x = 0; x < video_w; ++x) {
			int src_x = (x * SCREENWIDTH) / video_w;
			indextable[y * video_w + x] = src_y * SCREENWIDTH + src_x;
		}
	}
}

// Generated by ChatGPT-4.1 by GitHub Copilot: https://github.com/copilot/
static void genscalexytable(int video_w, int video_h) {
	int i;

	// Allocate lookup tables
	xtable  = suAllocMem(sizeof(uint16_t) * video_w, NULL);
	ytable  = suAllocMem(sizeof(uint16_t) * video_h, NULL);

	for (i = 0; i < video_h; i++) {
		ytable[i] = (i * SCREENWIDTH) / video_h;  // src_x
	}
	for (i = 0; i < video_w; i++) {
		// For each x in output, precompute src_y (rotated)
		xtable[i] = (((video_w - 1 - i) * SCREENHEIGHT) / video_w) * SCREENWIDTH;  // src_y
	}
}
#endif

void I_InitGraphicsHardwareSpecificCode(void)
{
	I_SetScreenMode(0x13);
	I_ReloadPalette();
	I_UploadNewPalette(0);

	__djgpp_nearptr_enable();

#if defined(FTR_GFX_ATI)
	_s_screen = (uint8_t *) pp_bitmap;
#elif defined(FTR_GFX_NVIDIA) || defined(FTR_GFX_DAL)
	INT32 result;
	_s_screen = uisAllocateMemory(SCREENWIDTH * SCREENHEIGHT, &result);
	if (result != RESULT_OK) {
		LOG("%s\n", "Cannot allocate: _s_screen!");
	}
#endif
	_fmemset(_s_screen, 0, SCREENWIDTH * SCREENHEIGHT);

#if defined(FTR_GFX_NVIDIA) || defined(FTR_GFX_DAL)
	genscalexytable(VIDEO_W, VIDEO_H);
	genindextable(VIDEO_W, VIDEO_H);
#endif
}

static boolean drawStatusBar = true;

static void I_DrawBuffer(uint8_t __far* buffer)
{
#if 0
	uint8_t __far* src = buffer;
	uint8_t __far* dst = _s_screen;

	for (uint_fast8_t y = 0; y < SCREENHEIGHT - ST_HEIGHT; y++)
	{
		_fmemcpy(dst, src, SCREENWIDTH);
		dst += SCREENWIDTH;
		src += SCREENWIDTH;
	}

	if (drawStatusBar)
	{
		for (uint_fast8_t y = 0; y < ST_HEIGHT; y++)
		{
			_fmemcpy(dst, src, SCREENWIDTH);
			dst += SCREENWIDTH;
			src += SCREENWIDTH;
		}
	}
	drawStatusBar = true;
#endif

#if defined(FTR_GFX_ATI)
	appi_g->ahi.bitmap.image = (void *) buffer;
#endif

#if defined(FTR_GFX_NVIDIA) || defined(FTR_GFX_DAL)
#if !defined(NVIDIA_FULLSCREEN)
	for (int i = 0; i < SCREENWIDTH * SCREENHEIGHT; i++) {
		pp_bitmap[i] = doom_current_palette[buffer[i]];
	}
#else
#if defined(FULLSCREEN_240X320) || defined(FULLSCREEN_176X220) || defined(FTR_C650)
	for (int i = 0; i < VIDEO_W * VIDEO_H; ++i) {
		pp_bitmap[i] = doom_current_palette[buffer[indextable[i]]];
	}
#elif defined(FULLSCREEN_320X240) || defined(FULLSCREEN_220X176)
	for (int y = 0; y < VIDEO_H; ++y)
	{
		for (int x = 0; x < VIDEO_W; ++x)
		{
			// CCW 90deg: dest(x, y) <- src(y', x')
			// src index = xtable[x] + ytable[y]
			pp_bitmap[y * VIDEO_W + x] = doom_current_palette[buffer[xtable[x] + ytable[y]]];
		}
	}
#endif
#endif
#endif
}

void I_ShutdownGraphics(void)
{
	I_SetScreenMode(3);

#if defined(FTR_GFX_NVIDIA) || defined(FTR_GFX_DAL)
	suFreeMem(xtable);
	suFreeMem(ytable);
	uisFreeMemory(indextable);
#endif
}

static int8_t newpal;

//
// I_SetPalette
//
void I_SetPalette(int8_t pal)
{
	newpal = pal;
}

//
// I_FinishUpdate
//

#define NO_PALETTE_CHANGE 100

void I_FinishUpdate(void)
{
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}

	I_DrawBuffer(_s_screen);
}

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
//

#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

const uint8_t* colormap;

const uint8_t __far* source;
uint8_t __far* dest;

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

#if defined C_ONLY
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
#else
void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count);
#endif

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

void R_DrawColumnWall(const draw_column_vars_t *dcvars)
{
	R_DrawColumnSprite(dcvars);
}

#if defined C_ONLY
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
#else
void R_DrawColumnFlat2(uint8_t col, uint8_t dontcare, int16_t count);
#endif

void R_DrawColumnFlat(uint8_t col, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	dest = _s_screen + (dcvars->yl * SCREENWIDTH) + (dcvars->x * 4 * 60 / VIEWWINDOWWIDTH);

	R_DrawColumnFlat2(col, col, count);
}

#define FUZZOFF 120 /* SCREENWIDTH / 2 so it fits in an int8_t */
#define FUZZTABLE 50

static const int8_t fuzzoffset[FUZZTABLE] =
{
	FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
	FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
	FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
	FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF
};

//
// Framebuffer postprocessing.
// Creates a fuzzy image by copying pixels
//  from adjacent ones to left and right.
// Used with an all black colormap, this
//  could create the SHADOW effect,
//  i.e. spectres and invisible players.
//
void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
	int16_t dc_yl = dcvars->yl;
	int16_t dc_yh = dcvars->yh;

	// Adjust borders. Low...
	if (dc_yl <= 0)
		dc_yl = 1;

	// .. and high.
	if (dc_yh >= VIEWWINDOWHEIGHT - 1)
		dc_yh = VIEWWINDOWHEIGHT - 2;

	int16_t count = (dc_yh - dc_yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	colormap = &fullcolormap[6 * 256];

	uint8_t __far* dest = _s_screen + (dc_yl * SCREENWIDTH) + (dcvars->x * 4 * 60 / VIEWWINDOWWIDTH);

	static int16_t fuzzpos = 0;

	do
	{
		R_DrawColumnPixel(dest, &dest[fuzzoffset[fuzzpos] * 2], 0);
		dest += SCREENWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while(--count);
}

#if !defined FLAT_SPAN
inline static void R_DrawSpanPixel(uint32_t __far* dest, const byte __far* source, const byte __far* colormap, uint32_t position)
{
	uint16_t color = colormap[source[((position >> 4) & 0x0fc0) | (position >> 26)]];
	color = color | (color << 8);

	uint16_t __far* d = (uint16_t __far*) dest;
	*d++ = color;
	*d   = color;
}

void R_DrawSpan(uint16_t y, uint16_t x1, uint16_t x2, const draw_span_vars_t *dsvars)
{
	uint16_t count = (x2 - x1);

	const byte __far* source   = dsvars->source;
	const byte __far* colormap = dsvars->colormap;

	uint32_t __far* dest = (uint32_t __far*)(_s_screen + (y * SCREENWIDTH) + (x1 << 2));

	const uint32_t step = dsvars->step;
	uint32_t position = dsvars->position;

	uint16_t l = (count >> 4);

	while (l--)
	{
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;

		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;

		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;

		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
    }

	switch (count & 15)
	{
		case 15:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case 14:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case 13:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case 12:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case 11:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case 10:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  9:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  8:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  7:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  6:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  5:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  4:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  3:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  2:    R_DrawSpanPixel(dest, source, colormap, position); dest++; position+=step;
		case  1:    R_DrawSpanPixel(dest, source, colormap, position);
	}
}
#endif

//
// V_ClearViewWindow
//
void V_ClearViewWindow(void)
{
	_fmemset(_s_screen, 0, SCREENWIDTH * (SCREENHEIGHT - ST_HEIGHT));
}

void V_InitDrawLine(void)
{
	// Do nothing
}

void V_ShutdownDrawLine(void)
{
	// Do nothing
}

//
// V_DrawLine()
//
// Draw a line in the frame buffer.
// Classic Bresenham w/ whatever optimizations needed for speed
//
// Passed the frame coordinates of line, and the color to be drawn
// Returns nothing
//
void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	int16_t dx = D_abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -D_abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	while (true)
	{
		_s_screen[y0 * SCREENWIDTH + x0] = color;

		if (x0 == x1 && y0 == y1)
			break;

		int16_t e2 = 2 * err;

		if (e2 >= dy)
		{
			err += dy;
			x0  += sx;
		}

		if (e2 <= dx)
		{
			err += dx;
			y0  += sy;
		}
	}
}

/*
 * V_DrawBackground tiles a 64x64 patch over the entire screen, providing the
 * background for the Help and Setup screens, and plot text between levels.
 * cphipps - used to have M_DrawBackground, but that was used the framebuffer
 * directly, so this is my code from the equivalent function in f_finale.c
 */
void V_DrawBackground(int16_t backgroundnum)
{
	/* erase the entire screen to a tiled background */
	const byte __far* src = W_GetLumpByNum(backgroundnum);

	for (int16_t y = 0; y < SCREENHEIGHT; y++)
	{
		for (int16_t x = 0; x < SCREENWIDTH; x += 64)
		{
			uint8_t __far* d = &_s_screen[y * SCREENWIDTH + x];
			const byte __far* s = &src[((y & 63) * 64)];

			size_t len = 64;

			if (SCREENWIDTH - x < 64)
				len = SCREENWIDTH - x;

			_fmemcpy(d, s, len);
		}
	}

	Z_ChangeTagToCache(src);
}

void V_DrawRaw(int16_t num, uint16_t offset)
{
	const uint8_t __far* lump = W_TryGetLumpByNum(num);

	if (lump != NULL)
	{
		uint16_t lumpLength = W_LumpLength(num);
		_fmemcpy(&_s_screen[offset], lump, lumpLength);
		Z_ChangeTagToCache(lump);
	}
	else
		W_ReadLumpByNum(num, &_s_screen[offset]);
}

void ST_Drawer(void)
{
	if (ST_NeedUpdate())
		ST_doRefresh();
	else
		drawStatusBar = false;
}

void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	y -= patch->topoffset;
	x -= patch->leftoffset;

	byte __far* desttop = _s_screen + (y * SCREENWIDTH) + x;

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++, desttop++)
	{
		const column_t __far* column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte __far* source = (const byte __far*)column + 3;
			byte __far* dest = desttop + (column->topdelta * SCREENWIDTH);

			uint16_t count = column->length;

			if (count == 7)
			{
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++;
			}
			else if (count == 3)
			{
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++;
			}
			else if (count == 5)
			{
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++;
			}
			else if (count == 6)
			{
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++;
			}
			else if (count == 2)
			{
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++;
			}
			else
			{
				while (count--)
				{
					*dest = *source++; dest += SCREENWIDTH;
				}
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}
	}
}

void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	static const int32_t   DX  = (((int32_t)SCREENWIDTH)<<FRACBITS) / SCREENWIDTH_VGA;
	static const int16_t   DXI = ((((int32_t)SCREENWIDTH_VGA)<<FRACBITS) / SCREENWIDTH) >> 8;
	static const int32_t   DY  = ((((int32_t)SCREENHEIGHT)<<FRACBITS)+(FRACUNIT-1)) / SCREENHEIGHT_VGA;
	static const int16_t   DYI = ((((int32_t)SCREENHEIGHT_VGA)<<FRACBITS) / SCREENHEIGHT) >> 8;

	y -= patch->topoffset;
	x -= patch->leftoffset;

	const int16_t left   = ( x * DX ) >> FRACBITS;
	const int16_t right  = ((x + patch->width)  * DX) >> FRACBITS;
	const int16_t bottom = ((y + patch->height) * DY) >> FRACBITS;

	uint16_t   col = 0;

	for (int16_t dc_x = left; dc_x < right; dc_x++, col += DXI)
	{
		if (dc_x < 0)
			continue;
		else if (dc_x >= SCREENWIDTH)
			break;

		const column_t __far* column = (const column_t __far*)((const byte __far*)patch + (uint16_t)patch->columnofs[col >> 8]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			byte __far* dest = _s_screen + (dc_yl * SCREENWIDTH) + dc_x;

			int16_t frac = 0;

			const byte __far* source = (const byte __far*)column + 3;

			int16_t count = dc_yh - dc_yl;
			while (count--)
			{
				*dest = source[frac >> 8];
				dest += SCREENWIDTH;
				frac += DYI;
			}

			column = (const column_t __far*)((const byte __far*)column + column->length + 4);
		}
	}
}

static uint16_t __far* frontbuffer;
static  int16_t __far* wipe_y_lookup;

void wipe_StartScreen(void)
{
	frontbuffer = Z_TryMallocStatic(SCREENWIDTH * SCREENHEIGHT);
	if (frontbuffer)
	{
		// copy back buffer to front buffer
		_fmemcpy(frontbuffer, _s_screen, SCREENWIDTH * SCREENHEIGHT);
	}
}

static boolean wipe_ScreenWipe(int16_t ticks)
{
	boolean done = true;

	uint16_t __far* backbuffer = (uint16_t __far*)_s_screen;

	while (ticks--)
	{
		I_DrawBuffer((uint8_t __far*)frontbuffer);

#if defined(FTR_GFX_ATI)
		ATI_Driver_Flush((APPLICATION_T *) appi_g);
#elif defined(FTR_GFX_NVIDIA)
		Nvidia_Driver_Flush((APPLICATION_T *) appi_g);
#elif defined(FTR_GFX_NVIDIA)
		DAL_Driver_Flush((APPLICATION_T *) appi_g);
#endif

		for (int16_t i = 0; i < SCREENWIDTH / 2; i++)
		{
			if (wipe_y_lookup[i] < 0)
			{
				wipe_y_lookup[i]++;
				done = false;
				continue;
			}

			// scroll down columns, which are still visible
			if (wipe_y_lookup[i] < SCREENHEIGHT)
			{
				/* cph 2001/07/29 -
				 *  The original melt rate was 8 pixels/sec, i.e. 25 frames to melt
				 *  the whole screen, so make the melt rate depend on SCREENHEIGHT
				 *  so it takes no longer in high res
				 */
				int16_t dy = (wipe_y_lookup[i] < 16) ? wipe_y_lookup[i] + 1 : SCREENHEIGHT / 25;
				// At most dy shall be so that the column is shifted by SCREENHEIGHT (i.e. just invisible)
				if (wipe_y_lookup[i] + dy >= SCREENHEIGHT)
					dy = SCREENHEIGHT - wipe_y_lookup[i];

				uint16_t __far* s = &frontbuffer[i] + ((SCREENHEIGHT - dy - 1) * (SCREENWIDTH / 2));

				uint16_t __far* d = &frontbuffer[i] + ((SCREENHEIGHT - 1) * (SCREENWIDTH / 2));

				// scroll down the column. Of course we need to copy from the bottom... up to
				// SCREENHEIGHT - yLookup - dy

				for (int16_t j = SCREENHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					*d = *s;
					d += -(SCREENWIDTH / 2);
					s += -(SCREENWIDTH / 2);
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[i]  + wipe_y_lookup[i] * SCREENWIDTH / 2;
				d = &frontbuffer[i] + wipe_y_lookup[i] * SCREENWIDTH / 2;

				for (int16_t j = 0 ; j < dy; j++)
				{
					*d = *s;
					d += (SCREENWIDTH / 2);
					s += (SCREENWIDTH / 2);
				}

				wipe_y_lookup[i] += dy;
				done = false;
			}
		}
	}

	return done;
}

static void wipe_initMelt()
{
	wipe_y_lookup = Z_MallocStatic((SCREENWIDTH / 2) * sizeof(int16_t));

	// setup initial column positions (y<0 => not ready to scroll yet)
	wipe_y_lookup[0] = -(M_Random() % 16);
	for (int8_t i = 1; i < SCREENWIDTH / 2; i++)
	{
		int8_t r = (M_Random() % 3) - 1;

		wipe_y_lookup[i] = wipe_y_lookup[i - 1] + r;

		if (wipe_y_lookup[i] > 0)
			wipe_y_lookup[i] = 0;
		else if (wipe_y_lookup[i] == -16)
			wipe_y_lookup[i] = -15;
	}
}

//
// D_Wipe
//
// CPhipps - moved the screen wipe code from D_Display to here
// The screens to wipe between are already stored, this just does the timing
// and screen updating

void D_Wipe(void)
{
	if (!frontbuffer)
		return;

	wipe_initMelt();

	boolean done;
	int32_t wipestart = I_GetTime() - 1;

	do
	{
		int32_t nowtime;
		int16_t tics;
		do
		{
			nowtime = I_GetTime();
			tics = nowtime - wipestart;
		} while (!tics);

		wipestart = nowtime;
		done = wipe_ScreenWipe(tics);

		M_Drawer();                   // menu is drawn even on top of wipes

	} while (!done);

	Z_Free(frontbuffer);
	Z_Free(wipe_y_lookup);
}

void I_Quit_P2k(void) {
	appi_g->app.exit_status = TRUE;
}
