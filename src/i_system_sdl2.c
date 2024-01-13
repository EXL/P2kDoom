#include "doomtype.h"
#include "d_event.h"
#include "d_main.h"

#include "i_system_sdl2.h"

//GBA Keys
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

static unsigned short *backbuffer;
static unsigned short *frontbuffer;

static unsigned int vid_width = 0;
static unsigned int vid_height = 0;

static unsigned int screen_width = 0;
static unsigned int screen_height = 0;

static unsigned char* pb = NULL;
static unsigned char* pl = NULL;

static SDL_Window *window;
static SDL_Surface *video;
static SDL_Surface *surface;
static SDL_Renderer *render;
static SDL_Texture *texture;
static SDL_Color *palette_sdl;

void I_FinishUpdate_e32(const byte* srcBuffer, const byte* pallete, const unsigned int width, const unsigned int height)
{
	pb = (unsigned char*)srcBuffer;
	pl = (unsigned char*)pallete;

	for(int p = 0; p < 256; p++)
	{
		palette_sdl[p].r = pl[3*p];
		palette_sdl[p].g = pl[(3*p)+1];
		palette_sdl[p].b = pl[(3*p)+2];
		palette_sdl[p].a = 0xFF;
		// fprintf(stderr, "%d %d %d\n", palette_sdl[p].r, palette_sdl[p].g, palette_sdl[p].b);
	}
	SDL_SetPaletteColors(surface->format->palette, palette_sdl, 0, 256);

	//SDL_FillRect(surface, NULL, 0);
	//memcpy(surface->pixels, pb, screen_width * screen_height);
	SDL_BlitSurface(surface, NULL, video, NULL);
	SDL_UpdateTexture(texture, NULL, video->pixels, video->pitch);
	SDL_RenderCopy(render, texture, NULL, NULL);

	SDL_RenderPresent(render);
}

void I_SetPallete_e32(const byte* pallete)
{

}

void I_InitScreen_e32()
{
	//Gives 480px on a 5(mx) and 320px on a Revo.
	vid_width = screen_width = 240;

	vid_height = screen_height = 160;
}

void I_CreateBackBuffer_e32()
{
//	backbuffer = malloc(screen_width * screen_height);
	frontbuffer = malloc(screen_width * screen_height);

	window = SDL_CreateWindow(
		"P2kDoom",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_width, screen_height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if (window == NULL) {
		SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
		return;
	}

	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (render == NULL) {
		SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
		return;
	}

	video = SDL_CreateRGBSurface(0, screen_width, screen_height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (video == NULL) {
		SDL_Log("SDL_CreateRGBSurface (video) failed: %s", SDL_GetError());
		return;
	}

	surface = SDL_CreateRGBSurface(0, screen_width, screen_height, 8, 0x00, 0x00, 0x00, 0x00);
	if (surface == NULL) {
		SDL_Log("SDL_CreateRGBSurface (surface) failed: %s", SDL_GetError());
		return;
	}
	backbuffer = surface->pixels;

	texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888,
								SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
	if (texture == NULL) {
		SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
		return;
	}

	palette_sdl = malloc(sizeof(SDL_Color) * 256);

	unsigned short* bb = I_GetBackBuffer();

	memset(bb, 0, 240*160);

//	I_FinishUpdate_e32(NULL, NULL, 0, 0);

	bb = I_GetFrontBuffer();

	memset(bb, 0, 240*160);

//	I_FinishUpdate_e32(NULL, NULL, 0, 0);
}

//**************************************************************************************

void I_ProcessKeyEvents()
{
	SDL_Event event;
	event_t ev;
	ev.data1 = 0;

	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				ev.type = (event.type == SDL_KEYUP) ? ev_keyup : ev_keydown;
				switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_ESCAPE:
						ev.data1 = KEYD_START;
						break;
					case SDL_SCANCODE_RETURN:
						ev.data1 = KEYD_A;
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
					case SDL_SCANCODE_TAB:
						ev.data1 = KEYD_SELECT;
						break;
					case SDL_SCANCODE_SPACE:
						ev.data1 = KEYD_B;
						break;
					case SDL_SCANCODE_A:
						ev.data1 = KEYD_L;
						break;
					case SDL_SCANCODE_D:
						ev.data1 = KEYD_R;
						break;
					default:
						break;
				}
				ev.data2 = 0;
				ev.data3 = 0;

				if(ev.data1 != 0)
					D_PostEvent(&ev);
				break;
			case SDL_QUIT:
				I_Quit_e32();
				return;
			default:
				break;
		}
	}
}

unsigned short* I_GetBackBuffer()
{
	return backbuffer;
}

unsigned short* I_GetFrontBuffer()
{
	return frontbuffer;
}

//**************************************************************************************

#define MAX_MESSAGE_SIZE 1024

void I_Error (const char *error, ...)
{
	char msg[MAX_MESSAGE_SIZE];

	va_list v;
	va_start(v, error);

	vsprintf(msg, error, v);

	va_end(v);

	fprintf(stderr, "%s\n", msg);


	fflush( stderr );
	fflush( stdout );

	//fgets(msg, sizeof(msg), stdin);

	I_Quit_e32();
}

//**************************************************************************************

void I_Quit_e32()
{
//	free(backbuffer);
	free(frontbuffer);
	free(palette_sdl);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(video);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);

	SDL_Quit();

	exit(0);
}

//**************************************************************************************
