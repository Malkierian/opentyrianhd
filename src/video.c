/*
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "keyboard.h"
#include "opentyr.h"
#include "palette.h"
#include "video.h"
#include "video_scale.h"

#include <assert.h>
#include <zdkdisplay.h>
#include <zdkgl.h>


bool fullscreen_enabled = false;

SDL_Surface *VGAScreen, *VGAScreenSeg;
SDL_Surface *VGAScreen2;
SDL_Surface *game_screen, *scale_surface, *display_surface;
int scaleFlipCounter;

ZDK_SPRITE dispSprite;
HTEXTURE zdkDisplay;
char buffer[100];

static ScalerFunction scaler_function;

void init_video( void )
{
	if (SDL_WasInit(SDL_INIT_VIDEO))
		return;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
	{
		fprintf(stderr, "error: failed to initialize SDL video: %s\n", SDL_GetError());
		exit(1);
	}
	HRESULT hr;
	hr = ZDKDisplay_Initialize();
	if(FAILED(hr))
		printf("Initialize failed: %08x\n",scaleFlipCounter,hr);

	hr = ZDKDisplay_CreateTexture(320,200,&zdkDisplay);
	if(FAILED(hr))
		printf("CreateTexture failed: %08x\n",scaleFlipCounter,hr);

	VGAScreen = VGAScreenSeg = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	VGAScreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	game_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	
	scale_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 32, 0, 0, 0, 0);

	display_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 32, 0, 0, 0, 0);

	dispSprite.Color = 0xFFFFFFFF;

	dispSprite.Vertices[0].X = 0;
	dispSprite.Vertices[0].Y = 0;
	dispSprite.Vertices[0].U = 0;
	dispSprite.Vertices[0].V = 1;
	
	dispSprite.Vertices[1].X = 272;
	dispSprite.Vertices[1].Y = 0;
	dispSprite.Vertices[1].U = 0;
	dispSprite.Vertices[1].V = 0;
	
	dispSprite.Vertices[2].X = 272;
	dispSprite.Vertices[2].Y = 480;
	dispSprite.Vertices[2].U = 1;
	dispSprite.Vertices[2].V = 0;
	
	dispSprite.Vertices[3].X = 0;
	dispSprite.Vertices[3].Y = 480;
	dispSprite.Vertices[3].U = 1;
	dispSprite.Vertices[3].V = 1;

	printf("Filling rectangle...\n");
	SDL_FillRect(VGAScreen, NULL, 0);

	printf("Initializing scaler...\n");
	if (!init_scaler(scaler, fullscreen_enabled) &&  // try desired scaler and desired fullscreen state
	    !init_any_scaler(fullscreen_enabled) &&      // try any scaler in desired fullscreen state
	    !init_any_scaler(!fullscreen_enabled))       // try any scaler in other fullscreen state
	{
		exit(EXIT_FAILURE);
	}
}

int can_init_scaler( unsigned int new_scaler, bool fullscreen )
{
	return 32;
}

bool init_scaler( unsigned int new_scaler, bool fullscreen )
{
	scaler_function = scalers[0].scaler32;
	fullscreen_enabled = false;
	
	input_grab();
	
	JE_showVGA();
	return true;
}

bool init_any_scaler( bool fullscreen )
{
	return true;
}

void deinit_video( void )
{
	SDL_FreeSurface(VGAScreenSeg);
	SDL_FreeSurface(VGAScreen2);
	SDL_FreeSurface(game_screen);
	SDL_FreeSurface(display_surface);
	SDL_FreeSurface(scale_surface);
	
	HRESULT hr = ZDKDisplay_FreeTexture(zdkDisplay);
	if(FAILED(hr))
		printf("FreeTexture failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);
	ZDKDisplay_Cleanup();
}

void JE_clr256( SDL_Surface * screen)
{
	memset(screen->pixels, 0, screen->pitch * screen->h);
}
void JE_showVGA( void ) { scale_and_flip(VGAScreen); }

void scale_and_flip( SDL_Surface *src_surface )
{
	HRESULT hr;
	assert(src_surface->format->BitsPerPixel == 8);

	hr = ZDKDisplay_Clear(0);
	
	assert(scaler_function != NULL);
	scaler_function(src_surface, scale_surface);
	ZDK_RECT dispRect;
	dispRect.Left = 0;
	dispRect.Top = 0;
	dispRect.Right = 320;
	dispRect.Bottom = 200;
	Uint32 *dispSrc = (Uint32 *)scale_surface->pixels, srcA, srcB, srcG, srcR,
		*dispDest = (Uint32 *)display_surface->pixels;
	for(int i = 0; i < 320 * 200; i++)
	{
		srcA = *dispSrc;
		srcA ^= 0xFF000000;
		*dispDest = srcA;
		dispDest ++;
		dispSrc ++;
	}

	hr = ZDKDisplay_BeginScene();
	if(FAILED(hr))
		printf("BeginScene failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);

	hr = ZDKDisplay_SetTextureData(zdkDisplay, &dispRect, display_surface->pixels, 4*320*200);
	if(FAILED(hr))
		printf("SetTextureData failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);

	hr = ZDKDisplay_SetTexture(zdkDisplay);
	if(FAILED(hr))
		printf("SetTexture failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);

	hr = ZDKDisplay_DrawSprites(&dispSprite, 1);
	if(FAILED(hr))
		printf("DrawSprites failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);

	hr = ZDKDisplay_EndScene();
	if(FAILED(hr))
		printf("EndScene failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);

	hr = ZDKDisplay_Present();
	if(FAILED(hr))
		printf("Present failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);
	scaleFlipCounter++;
}

// kate: tab-width 4; vim: set noet:
