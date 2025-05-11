/*
 * About:
 *   The "Yeti3D" is a port of heavy optimized 3D engine demo from GBA to Motorola P2K ELF application.
 *
 * Author:
 *   Derek J. Evans, EXL
 *
 * License:
 *   GPLv2
 *
 * Additional information:
 *   https://web.archive.org/web/20031204145215/http://www.theteahouse.com.au:80/gba/index.html
 *   https://sourceforge.net/projects/yeti3dpro/
 *   https://forum.motofan.ru/index.php?s=&showtopic=170514&view=findpost&p=1459600
 *
 * Application type:
 *   GUI + ATI, Nvidia + Java Heap + Videomode
 */

#include "doomdef.h"
#include "d_event.h"
#include "d_main.h"
#include "i_main.h"

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
#if defined(EP1) || defined(EP2)
#include <ati.h>
#elif defined(EM1) || defined(EM2)
#include <nvidia.h>
#endif

void I_FinishUpdate_e32(const byte* srcBuffer, const byte* pallete, const unsigned int width, const unsigned int height);

void I_SetPallete_e32(const byte* pallete);

unsigned short* I_GetBackBuffer();

unsigned short* I_GetFrontBuffer();

//void I_Error (const char *error, ...);

void I_Quit_e32();

void I_InitScreen_e32();

void I_CreateBackBuffer_e32();

//#include <stdarg.h>

#define TIMER_FAST_TRIGGER_MS             (1)
#if defined(FPS_15)
#define TIMER_FAST_UPDATE_MS              (1000 / 15) /* ~15 FPS. */
#elif defined(FPS_30)
#define TIMER_FAST_UPDATE_MS              (1000 / 30) /* ~30 FPS. */
#endif
#define KEYPAD_BUTTONS                    (8)

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

#if defined(EP1) || defined(EP2)
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
#elif defined(EM1) || defined(EM2)
typedef struct {
	GF_HANDLE gxHandle;
	GXRECT fb0_rect;
	UINT8 *fb0;
} APP_GFSDK_T;
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

#if defined(EP1) || defined(EP2)
	APP_AHI_T ahi;
#elif defined(EM1) || defined(EM2)
	APP_GFSDK_T gfsdk;
#endif
	APP_KEYBOARD_T keys;
	UINT32 timer_handle;
	UINT8 keyboard_volume_level;
	GRAPHIC_REGION_T dal_draw_region;
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

#if defined(EP1) || defined(EP2)
#if !defined(FTR_C650)
	static UINT32 ATI_Driver_Log(APPLICATION_T *app);
	static UINT32 ATI_Driver_Log_Memory(APPLICATION_T *app, AHIPIXFMT_T pixel_format);
#endif
static UINT32 ATI_Driver_Start(APPLICATION_T *app);
static UINT32 ATI_Driver_Stop(APPLICATION_T *app);
static UINT32 ATI_Driver_Flush(APPLICATION_T *app);
#elif defined(EM1) || defined(EM2)
static UINT32 Nvidia_Driver_Start(APPLICATION_T *app);
static UINT32 Nvidia_Driver_Stop(APPLICATION_T *app);
static UINT32 Nvidia_Driver_Flush(APPLICATION_T *app);
static UINT32 Nvidia_Driver_Fill(APPLICATION_T *app);
#endif

static UINT32 GFX_Draw_Start(APPLICATION_T *app);
static UINT32 GFX_Draw_Stop(APPLICATION_T *app);
static UINT32 GFX_Draw_Step(APPLICATION_T *app);

static UINT32 InitResourses(void);
static void FreeResourses(void);

static const char g_app_name[APP_NAME_LEN] = "P2kDoom";

#if defined(EP2)
static ldrElf g_app_elf;
#elif defined(EM1)
static ElfLoaderApp g_app_elf = { 0 };
#elif defined(EM2)
static ldrElf *g_app_elf = NULL;
#endif

static WCHAR g_res_file_path[FS_MAX_URI_NAME_LENGTH];

static UINT32 doom_current_palette[256];

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
	{ APP_STATE_MAIN, HandleStateEnter, HandleStateExit, g_state_main_hdls }
};

#if defined(EP1)
UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code) {
	UINT32 status;
	UINT32 ev_code_base;

	ev_code_base = ev_code;

	status = APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	u_atou(elf_path_uri, g_res_file_path);

	LdrStartApp(ev_code_base);

	return status;
}
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments) {
	UINT32 status;
	UINT32 ev_code_base;
	UINT32 reserve;

	if (ldrIsLoaded(g_app_name)) {
		cprint("P2kDoom: Error! Application has already been loaded!\n");
		return NULL;
	}

	status = RESULT_OK;
	ev_code_base = ldrRequestEventBase();
	reserve = ev_code_base + 1;
	reserve = ldrInitEventHandlersTbl(g_state_any_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_init_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_main_hdls, reserve);

	status |= APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	u_strcpy(g_res_file_path, uri);

	status |= ldrSendEvent(ev_code_base);
	g_app_elf.name = (char *) g_app_name;

	return (status == RESULT_OK) ? &g_app_elf : NULL;
}
#elif defined(EM1)
int _main(ElfLoaderApp ela) {
	UINT32 status;

	status = RESULT_OK;

	memcpy((void *) &g_app_elf, (void *) &ela, sizeof(ElfLoaderApp));

	status = APP_Register(&g_app_elf.evcode, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	u_strcpy(g_res_file_path, uri);

	LoaderShowApp(&g_app_elf);

	return RESULT_FAIL;
}
#elif defined(EM2)
UINT32 ELF_Entry(ldrElf *elf, WCHAR *arguments) {
	UINT32 status;
	UINT32 reserve;
	WCHAR *ptr;

	status = RESULT_OK;
	g_app_elf = elf;
	g_app_elf->name = (char *) g_app_name;

	if (ldrIsLoaded(g_app_elf->name)) {
		PFprintf("%s: Application already loaded.\n", g_app_elf->name);
		return RESULT_FAIL;
	}

	reserve = g_app_elf->evbase + 1;
	reserve = ldrInitEventHandlersTbl(g_state_any_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_init_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_main_hdls, reserve);

	status |= APP_Register(&g_app_elf->evbase, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);
	if (status == RESULT_OK) {
		PFprintf("%s: Application has been registered successfully.\n", g_app_elf->name);

		ptr = NULL;
		u_strcpy(g_res_file_path, L"file:/");
		ptr = g_res_file_path + u_strlen(g_res_file_path);
		DL_FsGetURIFromID(&g_app_elf->id, ptr);

		status |= ldrSendEvent(g_app_elf->evbase);
	} else {
		PFprintf("%s: Cannot register application.\n", g_app_elf->name);
	}

	return status;
}
#endif

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 0, 1, 1, 1, 0);

#if defined(EP1) || defined(EP2)
		app_instance->ahi.info_driver = NULL;
#elif defined(EM1) || defined(EM2)
		app_instance->gfsdk.gxHandle = (GF_HANDLE) &GxHandle;
		app_instance->gfsdk.fb0 = NULL;
#endif
		app_instance->bmp_width = 240;
		app_instance->bmp_height = 160;
		app_instance->p_bitmap = NULL;
		app_instance->timer_handle = 0;
		app_instance->keys.pressed = 0;
		app_instance->keys.released = 0;

		DL_AudGetVolumeSetting(PHONE, &app_instance->keyboard_volume_level);
		DL_AudSetVolumeSetting(PHONE, 0);

#if defined(EP1) || defined(EP2)
		status |= ATI_Driver_Start((APPLICATION_T *) app_instance);
#elif defined(EM1) || defined(EM2)
		status |= Nvidia_Driver_Start((APPLICATION_T *) app_instance);
#endif

		status |= APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);

#if defined(EP2)
		g_app_elf.app = (APPLICATION_T *) app_instance;
#elif defined(EM2)
		g_app_elf->app = &app_instance->app;
#endif
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	APP_ConsumeEv(ev_st, app);

	DeleteDialog(app);

	DL_AudSetVolumeSetting(PHONE, app_instance->keyboard_volume_level);

	status |= GFX_Draw_Stop(app);
	status |= SetLoopTimer(app, 0);
#if defined(EP1) || defined(EP2)
	status |= ATI_Driver_Stop(app);
#elif defined(EM1) || defined(EM2)
	status |= Nvidia_Driver_Stop(app);
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
	APP_INSTANCE_T *app_instance;
	IFACE_DATA_T iface_data;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;
	iface_data.port = app->port;

	if (app_instance->timer_handle != 0) {
		iface_data.handle = app_instance->timer_handle;
		status |= DL_ClkStopTimer(&iface_data);
	}

	if (period != 0) {
		DL_ClkStartCyclicalTimer(&iface_data, period, APP_TIMER_LOOP);
		status |= app_instance->timer_handle = iface_data.handle;
	}

	return status;
}

static UINT32 CheckKeyboard(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 key;
	APP_INSTANCE_T *app_instance;

	key = 0x00080000;

	app_instance = (APP_INSTANCE_T *) app;
	app_instance->keys.released = app_instance->keys.pressed;
	app_instance->keys.pressed = DL_KeyKjavaGetKeyState();

	while (key) {
		if ((app_instance->keys.released & key) != (app_instance->keys.pressed & key)) {
			if (app_instance->keys.pressed & key) {
				/* Key Pressed. */
				ProcessKeyboard(ev_st, app, key, TRUE);
			}
			if (app_instance->keys.released & key) {
				/* Key Released. */
				ProcessKeyboard(ev_st, app, key, FALSE);
			}
		}
		key >>= 1;
	}

	return RESULT_OK;
}

static UINT32 ProcessKeyboard(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 key, BOOL pressed) {
	event_t ev;
	ev.data1 = 0;
	// GBA Keys
	#define KEYD_A          1
	#define KEYD_B          2
	#define KEYD_L          3
	#define KEYD_R          4
	#define KEYD_UP         5
	#define KEYD_DOWN       6
	#define KEYD_LEFT       7
	#define KEYD_RIGHT      8
	#define KEYD_START      9
	#define KEYD_SELECT     10
#if defined(KEYS_PORTRAIT) || defined(FTR_C650)
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

	ev.type = (pressed) ? ev_keydown : ev_keyup;

	switch (key) {
		case MULTIKEY_0:
		case MULTIKEY_SOFT_LEFT:
			app->exit_status = TRUE;
			break;
		case MULTIKEY_1:
			ev.data1 = KEYD_START;
			break;
		case KK_2:
		case KK_UP:
			ev.data1 = KEYD_UP;
			break;
		case MULTIKEY_3:
			ev.data1 = KEYD_SELECT;
			break;
		case KK_4:
		case KK_LEFT:
			ev.data1 = KEYD_LEFT;
			break;
		case MULTIKEY_5:
		case MULTIKEY_JOY_OK:
			ev.data1 = KEYD_A;
			break;
		case KK_6:
		case KK_RIGHT:
			ev.data1 = KEYD_RIGHT;
			break;
		case MULTIKEY_7:
			ev.data1 = KEYD_L;
			break;
		case MULTIKEY_9:
			ev.data1 = KEYD_R;
			break;
		case KK_8:
		case KK_DOWN:
			ev.data1 = KEYD_DOWN;
			break;
		case MULTIKEY_STAR:
			ev.data1 = KEYD_A;
			break;
		case MULTIKEY_POUND:
			ev.data1 = KEYD_B;
			break;
		case MULTIKEY_SOFT_RIGHT:
			break;
		default:
			break;
	}

	ev.data2 = 0;
	ev.data3 = 0;

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
#if defined(EP1) || defined(EP2)
			ATI_Driver_Flush(app);
#elif defined(EM1) || defined(EM2)
			Nvidia_Driver_Flush(app);
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

unsigned short *backbuffer;
unsigned short *frontbuffer;

#if defined(EP1) || defined(EP2)
#if !defined(FTR_C650)
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

//#define NO_STRETCH
#if defined(NO_STRETCH)
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
	size_landscape_240x160.x = 240;
	size_landscape_240x160.y = 160;
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
#endif

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
			(display_mode.size.x < DISPLAY_WIDTH) ||
			(display_mode.size.y < DISPLAY_HEIGHT);

#if defined(NO_STRETCH)
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
#if defined(NO_STRETCH)
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

	backbuffer = suAllocMem(240 * 160, NULL);
	frontbuffer = suAllocMem(240 * 160, NULL);

	appi->ahi.bitmap.image = (UINT8 *) backbuffer;

//#if defined(JAVA_HEAP)
//	appi->ahi.bitmap.image = AmMemAllocPointer(appi->bmp_width * appi->bmp_height * 2);
//	if (!appi->ahi.bitmap.image) {
//#else
//	appi->ahi.bitmap.image = suAllocMem(appi->bmp_width * appi->bmp_height * 2, &result);
//	if (result != RESULT_OK) {
//#endif
//		LOG("%s\n", "Error: Cannot allocate screen buffer memory.");
//		return RESULT_FAIL;
//	}
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
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

#if defined(NO_STRETCH)
#if defined(FTR_L6)
	status |= ATI_Driver_Set_Display_Mode(app, AHIROT_0);
#else
	status |= ATI_Driver_Set_Display_Mode(app, (app_instance->is_CSTN_display) ? AHIROT_180 : AHIROT_0);
#endif
#endif

	if (app_instance->p_bitmap) {
		LOG("%s\n", "Free: ATI Bitmap memory.");
#if defined(JAVA_HEAP)
		AmMemFreePointer(app_instance->p_bitmap);
#else
		suFreeMem(app_instance->p_bitmap);
#endif
		app_instance->p_bitmap = NULL;
	}

	status |= AhiDevClose(app_instance->ahi.context);
	if (app_instance->ahi.info_driver) {
		LOG("%s\n", "Free: ATI Driver Info memory.");
		suFreeMem(app_instance->ahi.info_driver);
	}

	return status;
}

static UINT32 ATI_Driver_Flush(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

#if defined(NO_STRETCH)

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

#endif
	if (appi->is_CSTN_display) {
		AhiDispUpdate(appi->ahi.context, &appi->ahi.update_params);
	}

	return RESULT_OK;
}
#else
/* Pure DAL driver for C650-like phones. */
static UINT32 ATI_Driver_Start(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;
	UINT8 *display_bitmap;
	INT32 status;

	appi = (APP_INSTANCE_T *) app;
	display_bitmap = (UINT8 *) &display_source_buffer;

	appi->dal_draw_region.ulc.x = 0;
	appi->dal_draw_region.ulc.y = 0;
	appi->dal_draw_region.lrc.x = DISPLAY_WIDTH - 1;
	appi->dal_draw_region.lrc.y = DISPLAY_HEIGHT - 1;

	memset(display_bitmap, 0x00, DISPLAY_WIDTH * DISPLAY_HEIGHT * DISPLAY_BYTESPP);
	DAL_UpdateDisplayRegion(&appi->dal_draw_region, (UINT16 *) display_bitmap);

#if defined(VIEW_96X64)
	appi->dal_draw_region.ulc.x = (DISPLAY_WIDTH / 2) - (YETI_DISPLAY_WIDTH / 2);;
	appi->dal_draw_region.ulc.y = (DISPLAY_HEIGHT / 2) - (YETI_DISPLAY_HEIGHT / 2);
	appi->dal_draw_region.lrc.x = ((DISPLAY_WIDTH / 2) - (YETI_DISPLAY_WIDTH / 2)) + (YETI_DISPLAY_WIDTH - 1);
	appi->dal_draw_region.lrc.y = ((DISPLAY_HEIGHT / 2) - (YETI_DISPLAY_HEIGHT / 2)) + (YETI_DISPLAY_HEIGHT - 1);
#endif

	appi->ahi.bitmap.image = uisAllocateMemory(appi->bmp_width * appi->bmp_height * 2, &status);

	return status;
}

static UINT32 ATI_Driver_Stop(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	uisFreeMemory(appi->ahi.bitmap.image);

	return RESULT_OK;
}

static UINT32 ATI_Driver_Flush(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

//	DAL_UpdateDisplayRegion(&appi->dal_draw_region, display_bitmap);
	DAL_UpdateRectangleDisplayRegion(&appi->dal_draw_region, (UINT16 *) appi->p_bitmap, DISPLAY_MAIN);
//	DAL_WriteDisplayRegion(&appi->dal_draw_region, display_bitmap, DISPLAY_MAIN, FALSE);

	return RESULT_OK;
}
#endif // !defined(FTR_C650)
#elif defined(EM1) || defined(EM2)
static UINT32 Nvidia_Driver_Start(APPLICATION_T *app) {
	INT32 result;
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	GRAPHIC_POINT_T point;

	result = RESULT_OK;
	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	backbuffer = suAllocMem(240 * 160, NULL);
	frontbuffer = suAllocMem(240 * 160, NULL);

	app_instance->gfsdk.fb0_rect.x = 0;
	app_instance->gfsdk.fb0_rect.y = 0;

	UIS_CanvasGetDisplaySize(&point);
	app_instance->gfsdk.fb0_rect.w = point.x + 1;
	app_instance->gfsdk.fb0_rect.h = point.y + 1;
	app_instance->gfsdk.fb0 = uisAllocateMemory(
		app_instance->gfsdk.fb0_rect.w * app_instance->gfsdk.fb0_rect.h * 2, &result
	);
	if (result != RESULT_OK) {
		LOG("Cannot allocate '%d' bytes of memory for fill screen!\n",
			app_instance->gfsdk.fb0_rect.w * app_instance->gfsdk.fb0_rect.h * 2);
		status = RESULT_FAIL;
	}
	memclr(app_instance->gfsdk.fb0, app_instance->gfsdk.fb0_rect.w * app_instance->gfsdk.fb0_rect.h * 2);
	Nvidia_Driver_Flush(app);
	uisFreeMemory(app_instance->gfsdk.fb0);

	app_instance->gfsdk.fb0_rect.w = point.x + 1;
#if defined(NVIDIA_FULLSCREEN)
	app_instance->gfsdk.fb0_rect.h = point.y + 1;
#else
	app_instance->gfsdk.fb0_rect.h = 160;
#endif

//	app_instance->gfsdk.fb0_rect.x = 0;
//	app_instance->gfsdk.fb0_rect.y = 0;
//	app_instance->gfsdk.fb0_rect.w = 176;
//	app_instance->gfsdk.fb0_rect.h = 220;

	app_instance->gfsdk.fb0 = uisAllocateMemory(app_instance->gfsdk.fb0_rect.w * app_instance->gfsdk.fb0_rect.h * 2, &result);
	if (result != RESULT_OK) {
		LOG("Cannot allocate '%d' bytes of memory for screen!\n", app_instance->gfsdk.fb0_rect.w * app_instance->gfsdk.fb0_rect.h * 2);
		status = RESULT_FAIL;
	}

#if !defined(NVIDIA_FULLSCREEN)
	app_instance->gfsdk.fb0_rect.y = 320 / 2 - 160 / 2;
#endif

	return status;
}

static UINT32 Nvidia_Driver_Stop(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	uisFreeMemory(app_instance->gfsdk.fb0);

	return status;
}

static UINT32 Nvidia_Driver_Flush(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	GFGxCopyColorBitmap(
		app_instance->gfsdk.gxHandle,
		app_instance->gfsdk.fb0_rect.x, app_instance->gfsdk.fb0_rect.y,
		app_instance->gfsdk.fb0_rect.w, app_instance->gfsdk.fb0_rect.h,
#if defined(NVIDIA_FULLSCREEN)
		app_instance->gfsdk.fb0_rect.x, app_instance->gfsdk.fb0_rect.y,
#else
		app_instance->gfsdk.fb0_rect.x, 0,
#endif
		app_instance->gfsdk.fb0_rect.w * 2,
		app_instance->gfsdk.fb0
	);

	return status;
}
#endif

//#define MEMORY_MANUAL_ALLOCATION
#if defined(MEMORY_MANUAL_ALLOCATION)
#define MEMORY_ATTEMPTS (64)
#define MEMORY_END_BLOCK_SIZE (4096)
typedef struct {
	void *ptr;
	UINT32 size;
} MEMORY_BLOCK_ALLOCATED;
static MEMORY_BLOCK_ALLOCATED mem_blocks[MEMORY_ATTEMPTS];
static int mem_total_size;
static int mem_block_count;
static void Allocate_Memory_Blocks(int start_size) {
	INT32 status;
	int i, size, block_idx;

	status = RESULT_OK;
	mem_total_size = 0;
	mem_block_count = 0;
	block_idx = 0;
	size = start_size;

	for (i = 0; i < MEMORY_ATTEMPTS; ++i) {
		mem_blocks[i].ptr = NULL;
		mem_blocks[i].size = 0;
	}
	for (i = 0; i < MEMORY_ATTEMPTS; ++i) {
		mem_blocks[block_idx].ptr = suAllocMem(size, &status);
		if (status != RESULT_OK) {
			LOG("C=%d E=%d T=%d\n", i+1, size, mem_total_size);
			size /= 2;
			if (size < MEMORY_END_BLOCK_SIZE) {
				break;
			}
		} else {
			mem_total_size += size;
			mem_blocks[block_idx].size = size;
			LOG("C=%d A=%d T=%d P=0x%X\n", i+1, size, mem_total_size, mem_blocks[block_idx]);
			block_idx++;
		}
	}

	LOG("\n\n%s\n\n", "=== MEMORY BLOCKS STATISTIC TABLE ===");
	for (i = 0; i < block_idx; ++i) {
		LOG("Memory Block #%d: %d bytes, %d KiB, 0x%X\n",
			i+1, mem_blocks[i].size, mem_blocks[i].size / 1024, mem_blocks[i].ptr);
	}
	LOG("Total Memory: %d bytes, %d KiB.\n\n", mem_total_size, mem_total_size / 1024);

	mem_block_count = block_idx + 1;
}

static void Free_Memory_Blocks(void) {
	int i;

	for (i = 0; i < mem_block_count - 1; ++i) {
		if (mem_blocks[i].ptr) {
			LOG("Free Memory Block #%d: %d bytes, %d KiB, 0x%X\n",
				i+1, mem_blocks[i].size, mem_blocks[i].size / 1024, mem_blocks[i].ptr);
			suFreeMem(mem_blocks[i].ptr);
		}
	}
}
#endif

static UINT16 *pp_bitmap;
static APP_INSTANCE_T *appi_g;
static UINT8 *palette_g;

static UINT32 GFX_Draw_Start(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

#if defined(EP1) || defined(EP2)
	appi->p_bitmap = (UINT8 *) appi->ahi.bitmap.image;
#elif defined(EM1) || defined(EM2)
	appi->p_bitmap = (UINT8 *) appi->gfsdk.fb0;
#endif

	pp_bitmap = (UINT16 *) appi->p_bitmap;
	appi_g = appi;

	init_main(0, NULL);

	return RESULT_OK;
}

static UINT32 GFX_Draw_Stop(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	LOG("%s\n", "STOP");

	return RESULT_OK;
}

static UINT32 GFX_Draw_Step(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	D_DoomStep();

	return RESULT_OK;
}

void I_CopyBackBufferToFrontBuffer(void)
{
	UINT16 i;
	UINT32 *src, *dst;
	src = (UINT32 *) I_GetBackBuffer();
	dst = (UINT32 *) I_GetFrontBuffer();
	for (i = 0; i < SCREENWIDTH * SCREENHEIGHT / 2; i++)
		*dst++ = *src++;
}

void I_DrawBuffer(const byte* srcBuffer)
{

#if defined(EP1) || defined(EP2)
	appi_g->ahi.bitmap.image = (void *) srcBuffer;
#elif defined(EM1) || defined(EM2)

#if !defined(NVIDIA_FULLSCREEN)
	for (int i = 0; i < 240*160; i++) {
		UINT8 r = palette_g[srcBuffer[i] * 3];
		UINT8 g = palette_g[srcBuffer[i] * 3 + 1];
		UINT8 b = palette_g[srcBuffer[i] * 3 + 2];
		pp_bitmap[i] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
	}
#else
	#if defined(NVIDIA_FULLSCREEN_PORTRAIT_240X320)
		for (int i = 0; i < (SCREENWIDTH * 2) * SCREENHEIGHT; i++) {
			UINT8 r = palette_g[srcBuffer[i] * 3];
			UINT8 g = palette_g[srcBuffer[i] * 3 + 1];
			UINT8 b = palette_g[srcBuffer[i] * 3 + 2];
			unsigned short color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

			// Calculate the corresponding destination indices
			INT32 x = i % (SCREENWIDTH * 2);
			INT32 y = i / (SCREENWIDTH * 2);
			INT32 destIndex1 = (y * 2) * (SCREENWIDTH * 2) + x;      // First row of the 2px height
			INT32 destIndex2 = (y * 2 + 1) * (SCREENWIDTH * 2) + x;  // Second row of the 2px height

			// Set the color for both rows
			pp_bitmap[destIndex1] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
			pp_bitmap[destIndex2] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
		}
	#elif defined(NVIDIA_FULLSCREEN_PORTRAIT_176X220)
		#define WWW (176)
		#define HHH (220)
		// Fixed-point scaling factors (scaled by a factor of 256)
		const int scaleX = ((SCREENWIDTH * 2) << 8) / WWW;   // 240 * 256 / 176
		const int scaleY = ((SCREENHEIGHT) << 8) / HHH; // 160 * 256 / 220

		// Single loop for all destination pixels
		for (int destIndex = 0; destIndex < WWW * HHH; destIndex++) {
			// Compute destination coordinates (x, y)
			int destX = destIndex % WWW;
			int destY = destIndex / WWW;

			// Calculate the corresponding source pixel using fixed-point math
			int srcX = (destX * scaleX) >> 8; // Divide by 256
			int srcY = (destY * scaleY) >> 8; // Divide by 256
			int srcIndex = srcY * (SCREENWIDTH * 2) + srcX;

			// Get the RGB values from the palette
			unsigned char r = palette_g[srcBuffer[srcIndex] * 3];
			unsigned char g = palette_g[srcBuffer[srcIndex] * 3 + 1];
			unsigned char b = palette_g[srcBuffer[srcIndex] * 3 + 2];

			// Write the color to the destination buffer
			// Convert to 16-bit color format
			pp_bitmap[destIndex] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
		}
	#elif defined(NVIDIA_FULLSCREEN_LANDSCAPE)
#if defined(FTR_E770V)
		#define WWW (176)
		#define HHH (220)
#else
		#define WWW (240)
		#define HHH (320)
#endif
		// Calculate scaling factors (integer math, avoiding float)
		// Using fixed-point 16.16 format for precision
		int scaleX = ((SCREENWIDTH * 2) << 16) / HHH; // Scale factor for X (source width to destination height)
		int scaleY = ((SCREENHEIGHT) << 16) / WWW; // Scale factor for Y (source height to destination width)

		// Iterate over all destination pixels
		for (int i = 0; i < WWW * HHH; i++) {
			int x = i % WWW; // x-coordinate in destination
			int y = i / WWW; // y-coordinate in destination

			// Map destination (x, y) to source (srcX, srcY) with scaling and rotation
			int srcX = (y * scaleX) >> 16; // Scale and rotate
			int srcY = ((WWW - 1 - x) * scaleY) >> 16; // Scale and rotate

			// Ensure source coordinates are within bounds
			int srcIndex = srcY * (SCREENWIDTH * 2) + srcX; // Calculate source index

			// Fetch color from palette
			UINT8 r = palette_g[srcBuffer[srcIndex] * 3];
			UINT8 g = palette_g[srcBuffer[srcIndex] * 3 + 1];
			UINT8 b = palette_g[srcBuffer[srcIndex] * 3 + 2];

			pp_bitmap[i] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
		}
	#else
		#error "Unknown blitting method!"
	#endif
#endif

#endif

}

void I_DrawFrontBuffer(void)
{
	I_DrawBuffer((byte *) I_GetFrontBuffer());
#if defined(EP1) || defined(EP2)
	ATI_Driver_Flush((APPLICATION_T *) appi_g);
#elif defined(EM1) || defined(EM2)
	Nvidia_Driver_Flush((APPLICATION_T *) appi_g);
#endif
}

void I_FinishUpdate_e32(const byte* srcBuffer, const byte* palette, const unsigned int width, const unsigned int height)
{
	I_DrawBuffer((byte *) I_GetBackBuffer());
}

void I_Quit_e32() {

}

void I_SetPallete_e32(const byte* palette)
{
	palette_g = palette;
#if defined(EP1) || defined(EP2)
	for(int p = 0; p < 256; p++) {
		doom_current_palette[p] = ATI_565RGB(palette[3*p], palette[(3*p)+1], palette[(3*p)+2]);
	}
#endif
}

void I_InitScreen_e32()
{
	//Gives 480px on a 5(mx) and 320px on a Revo.
//	vid_width = screen_width = 240;

//	vid_height = screen_height = 160;
}

void I_CreateBackBuffer_e32()
{
//	backbuffer = suAllocMem(240 * 160, NULL);
//	frontbuffer = suAllocMem(240 * 160, NULL);

	unsigned short* bb = I_GetBackBuffer();

	memset(bb, 0, 240*160);

//	I_FinishUpdate_e32(NULL, NULL, 0, 0);

	bb = I_GetFrontBuffer();

	memset(bb, 0, 240*160);

//	I_FinishUpdate_e32(NULL, NULL, 0, 0);
}

void I_ProcessKeyEvents() {

}

unsigned short* I_GetBackBuffer()
{
	return backbuffer;
}

unsigned short* I_GetFrontBuffer()
{
	return frontbuffer;
}

#if 0
#define MAX_MESSAGE_SIZE 128

void I_Error (const char *error, ...)
{
	char msg[MAX_MESSAGE_SIZE];

//	va_list v;
//	va_start(v, error);

	sprintf(msg, "%s\n", error);

//	va_end(v);

	UINT32 written;
	UINT32 size = strlen(msg);
	FILE_HANDLE_T file = DL_FsOpenFile(L"/a/elf/doom_log.txt", FILE_APPEND_PLUS_MODE, 0x0E);
	if (file == FILE_HANDLE_INVALID) {
		return;
	}
	DL_FsWriteFile((void *) msg, size + 1, 1, file, &written);
	DL_FsCloseFile(file);

	//fgets(msg, sizeof(msg), stdin);

	I_Quit_e32();
}
#endif

#if defined(EP1) || defined(EP2)
void _abort(void) {
       LOG("%s\n", "Abort!");
}

void *_memchr(const void *s, unsigned char c, size_t n) {
    if (n != 0) {
        const unsigned char *p = s;

        do {
            if (*p++ == c)
                return ((void *)(p - 1));
        } while (--n != 0);
    }
    return (NULL);
}

void __aeabi_memcpy4(void *dest, const void *src, size_t n) {
    memcpy(dest, src, n);
}
#endif

#if 0
INT64 __aeabi_lmul(INT64 a, INT64 b) {
    return a * b;
}

INT32 __aeabi_idiv(INT32 numerator, INT32 denominator) {
    // Handle division by zero
    if (denominator == 0) {
        // Division by zero is undefined; handle as appropriate for your use case
        // For example, return 0, raise an error, or halt
        return 0; // Placeholder behavior
    }

    // Determine the sign of the result
    BOOL negative_result = (numerator < 0) ^ (denominator < 0);

    // Perform division using absolute values
    INT32 abs_numerator = numerator < 0 ? -numerator : numerator;
    INT32 abs_denominator = denominator < 0 ? -denominator : denominator;

    INT32 result = abs_numerator / abs_denominator;

    // Apply the sign to the result
    return negative_result ? -result : result;
}

INT64 __aeabi_ldivmod(INT64 numerator, INT64 denominator) {
    // Handle division by zero
    if (denominator == 0) {
        // Division by zero is undefined; handle appropriately
        // For this example, we return 0 as placeholder behavior
        return 0;
    }

    // Compute quotient and remainder
    INT64 quotient = numerator / denominator;
    INT64 remainder = numerator % denominator;

    // Pack the remainder into the return value
    // According to the ARM EABI, the quotient is returned in the primary return register
    // and the remainder is stored in a secondary register.
    // If you're writing this for a bare-metal system, you may need to handle register passing explicitly.

    // For simulation purposes, we store the remainder in a global or return structure.
    // NOTE: In actual ARM EABI, this would involve register usage, which is handled by the compiler.
    return quotient; // Quotient is returned
}

INT32 __aeabi_idivmod(INT32 numerator, INT32 denominator) {
    // Handle division by zero
    if (denominator == 0) {
        // Division by zero is undefined; handle appropriately
        // For this example, return 0 as placeholder
        return 0;
    }

    // Compute quotient and remainder
    INT32 quotient = numerator / denominator;
    INT32 remainder = numerator % denominator;

    // In ARM EABI, the quotient is returned in the primary return register,
    // and the remainder is returned in a secondary register.
    // If you're writing for a bare-metal system, you may need to handle register
    // passing explicitly, but in a C implementation, you can return a structure.

    // For simplicity, we will encode the remainder in a custom structure or global variable.
    // NOTE: In real ARM EABI, the compiler would handle this correctly.

    return quotient; // Quotient is returned
}

UINT32 __aeabi_uidiv(UINT32 numerator, UINT32 denominator) {
    // Handle division by zero
    if (denominator == 0) {
        // Division by zero is undefined; handle appropriately
        // For this example, return 0 as placeholder
        return 0;
    }

    // Perform the division
    return numerator / denominator;
}

// Function to perform unsigned integer division and modulo
UINT32 __aeabi_uidivmod(UINT32 numerator, UINT32 denominator) {
    // Handle division by zero
    if (denominator == 0) {
        // Division by zero is undefined; handle appropriately
        // For this example, return 0 as placeholder
        return 0;
    }

    // Compute quotient and remainder
    UINT32 quotient = numerator / denominator;
    UINT32 remainder = numerator % denominator;

    // In ARM EABI, the quotient is returned in the primary return register,
    // and the remainder is returned in a secondary register.
    // If you're writing this in a low-level environment, you may need
    // to handle register passing explicitly.
    // In C, you can use a structure or global variable to simulate this.

    // For simplicity, this returns only the quotient.
    // Use additional logic to return the remainder where required.
    return quotient; // Quotient is returned
}

UINT64 __aeabi_uldivmod(UINT64  numerator, UINT64  denominator) {
    // Handle division by zero
    if (denominator == 0) {
        // Division by zero is undefined; handle appropriately
        // For this example, return 0 as a placeholder
        return 0;
    }

    // Compute quotient and remainder
    UINT64 quotient = numerator / denominator;
    UINT64 remainder = numerator % denominator;

    // In ARM EABI, the quotient is returned in the primary return register,
    // and the remainder is returned in a secondary register.
    // If you're writing this for a bare-metal system, you may need to handle register passing explicitly.
    // For simplicity, we return only the quotient here.

    return quotient; // Quotient is returned
}
#endif
