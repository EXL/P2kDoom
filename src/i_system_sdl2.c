#include "doomtype.h"
#include "d_event.h"
#include "d_main.h"

#include "i_system_sdl2.h"
#include "i_main.h"
#include "d_main.h"
#include "doomdef.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

void I_DrawBuffer(const byte* buffer)
{
//	surface->pixels = (void *) buffer;

#if 0
	int srcWidth = SCREENWIDTH * 2;  // Source width
	int srcHeight = SCREENHEIGHT; // Source height
	int destWidth = vid_width; // Destination width
	int destHeight = vid_height; // Destination height

	unsigned short *destPixels = (unsigned short *) surface->pixels;

	// Calculate scaling factors (integer math, avoiding float)
	// Using fixed-point 16.16 format for precision
	int scaleX = (srcWidth << 16) / destHeight; // Scale factor for X (source width to destination height)
	int scaleY = (srcHeight << 16) / destWidth; // Scale factor for Y (source height to destination width)

	// Iterate over all destination pixels
	for (int i = 0; i < destWidth * destHeight; i++) {
		int x = i % destWidth; // x-coordinate in destination
		int y = i / destWidth; // y-coordinate in destination

		// Map destination (x, y) to source (srcX, srcY) with scaling and rotation
		int srcX = (y * scaleX) >> 16; // Scale and rotate
		int srcY = ((destWidth - 1 - x) * scaleY) >> 16; // Scale and rotate

		// Ensure source coordinates are within bounds
		int srcIndex = srcY * srcWidth + srcX; // Calculate source index

		// Fetch color from palette
		unsigned char r = pl[buffer[srcIndex] * 3];
		unsigned char g = pl[buffer[srcIndex] * 3 + 1];
		unsigned char b = pl[buffer[srcIndex] * 3 + 2];

		// Convert to RGB565 format
		unsigned short rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

		// Write to destination
		destPixels[i] = rgb565;
	}

	for (int i = 0; i < (SCREENWIDTH * 2) * SCREENHEIGHT; i++) {
		unsigned char r = pl[buffer[i] * 3];
		unsigned char g = pl[buffer[i] * 3 + 1];
		unsigned char b = pl[buffer[i] * 3 + 2];

		// Calculate the corresponding destination indices
		int x = i % (SCREENWIDTH * 2);
		int y = i / (SCREENWIDTH * 2);
		int destIndex1 = (y * 2) * (SCREENWIDTH * 2) + x;      // First row of the 2px height
		int destIndex2 = (y * 2 + 1) * (SCREENWIDTH * 2) + x;  // Second row of the 2px height

		// Set the color for both rows
		((unsigned short *) surface->pixels)[destIndex1] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
		((unsigned short *) surface->pixels)[destIndex2] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
	}

	// Constants for source and destination dimensions
	const int srcWidth = SCREENWIDTH * 2;
	const int srcHeight = SCREENHEIGHT;
	const int destWidth = vid_width;
	const int destHeight = vid_height;

	// Fixed-point scaling factors (scaled by a factor of 256)
	const int scaleX = (srcWidth << 8) / destWidth;   // 240 * 256 / 176
	const int scaleY = (srcHeight << 8) / destHeight; // 160 * 256 / 220

	// Total number of pixels in the destination
	const int destTotalPixels = destWidth * destHeight;

	// Single loop for all destination pixels
	for (int destIndex = 0; destIndex < destTotalPixels; destIndex++) {
		// Compute destination coordinates (x, y)
		int destX = destIndex % destWidth;
		int destY = destIndex / destWidth;

		// Calculate the corresponding source pixel using fixed-point math
		int srcX = (destX * scaleX) >> 8; // Divide by 256
		int srcY = (destY * scaleY) >> 8; // Divide by 256
		int srcIndex = srcY * srcWidth + srcX;

		// Get the RGB values from the palette
		unsigned char r = pl[buffer[srcIndex] * 3];
		unsigned char g = pl[buffer[srcIndex] * 3 + 1];
		unsigned char b = pl[buffer[srcIndex] * 3 + 2];

		// Convert to 16-bit color format
		unsigned short color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

		// Write the color to the destination buffer
		((unsigned short *) surface->pixels)[destIndex] = color;
	}
#endif

	for (int i = 0; i < (SCREENWIDTH * 2) * SCREENHEIGHT; i++) {
		unsigned char r = pl[buffer[i] * 3];
		unsigned char g = pl[buffer[i] * 3 + 1];
		unsigned char b = pl[buffer[i] * 3 + 2];
		((unsigned short *) surface->pixels)[i] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
	}

	SDL_BlitSurface(surface, NULL, video, NULL);
	SDL_UpdateTexture(texture, NULL, video->pixels, video->pitch);
	SDL_RenderCopy(render, texture, NULL, NULL);

	SDL_RenderPresent(render);
}

void I_FinishUpdate_e32(const byte* srcBuffer, const byte* pallete, const unsigned int width, const unsigned int height)
{
	I_DrawBuffer((byte *) I_GetBackBuffer());
}

void I_DrawFrontBuffer(void)
{
	I_DrawBuffer((byte *) I_GetFrontBuffer());
}

void I_SetPallete_e32(const byte* pallete)
{
	pl = (unsigned char*)pallete;
}

void I_InitScreen_e32()
{
	//Gives 480px on a 5(mx) and 320px on a Revo.
	vid_width = screen_width = 240;
	vid_height = screen_height = 160;
}

void I_CreateBackBuffer_e32()
{
	backbuffer = malloc(screen_width * screen_height);
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

	surface = SDL_CreateRGBSurface(0, screen_width, screen_height, 16, 0xF800, 0x07E0, 0x001F, 0x0000);
	if (surface == NULL) {
		SDL_Log("SDL_CreateRGBSurface (surface) failed: %s", SDL_GetError());
		return;
	}

	texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888,
								SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
	if (texture == NULL) {
		SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
		return;
	}

	palette_sdl = malloc(sizeof(SDL_Color) * 256);

	unsigned short* bb = I_GetBackBuffer();

	memset(bb, 0, screen_width*screen_height);

	bb = I_GetFrontBuffer();

	memset(bb, 0, screen_width*screen_height);
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
					case SDL_SCANCODE_Q:
						ev.data1 = KEYD_START;
						break;
					case SDL_SCANCODE_Z:
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
					case SDL_SCANCODE_W:
						ev.data1 = KEYD_SELECT;
						break;
					case SDL_SCANCODE_X:
						ev.data1 = KEYD_B;
						break;
					case SDL_SCANCODE_A:
						ev.data1 = KEYD_L;
						break;
					case SDL_SCANCODE_S:
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

	fprintf(stderr, "%s", msg);


	fflush( stderr );
	fflush( stdout );

	//fgets(msg, sizeof(msg), stdin);

//	I_Quit_e32();
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

/*
 * I_Read
 *
 * cph 2001/11/18 - wrapper for read(2) which handles partial reads and aborts
 * on error.
 */
void I_Read(int fd, void* vbuf, size_t sz)
{
	unsigned char* buf = vbuf;

	while (sz) {
		int rc = read(fd,buf,sz);
		if (rc <= 0) {
			I_Error("I_Read: read failed: %s", rc ? strerror(errno) : "EOF");
		}
		sz -= rc; buf += rc;
	}
}

/*
 * I_Filelength
 *
 * Return length of an open file.
 */

int I_Filelength(int handle)
{
	struct stat   fileinfo;
	if (fstat(handle,&fileinfo) == -1)
		I_Error("I_Filelength: %s",strerror(errno));
	return fileinfo.st_size;
}

int main(int argc, const char *const argv[]) {
	init_main(argc, argv);
	while ("DOOM is Rock")
		D_DoomStep();
	return 0;
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

void I_CopyBackBufferToFrontBuffer(void)
{
	uint16_t i;
	uint32_t *src, *dst;
	src = (uint32_t *) I_GetBackBuffer();
	dst = (uint32_t *) I_GetFrontBuffer();
	for (i = 0; i < SCREENWIDTH * SCREENHEIGHT / 2; i++)
		*dst++ = *src++;
}
