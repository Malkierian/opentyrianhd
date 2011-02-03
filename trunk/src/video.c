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
#include "input.h"
#include "keyboard.h"
#include "opentyr.h"
#include "palette.h"
#include "video.h"
#include "video_scale.h"
#include "varz.h"

#include <assert.h>

#include <zdkdisplay.h>
#include <zdkimage.h>


bool fullscreen_enabled = false;

SDL_Surface *VGAScreen, *VGAScreenSeg;
SDL_Surface *VGAScreen2;
SDL_Surface *game_screen, *scale_surface;
int scaleFlipCounter;

ZDK_SPRITE dispSprite;
HTEXTURE zdkDisplay;
HTEXTURE layout_texture;
HTEXTURE game_texture;
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

	hr = ZDKDisplay_CreateTexture(480,272,&layout_texture);
	if(FAILED(hr))
		printf("CreateTexture failed: %08x\n",scaleFlipCounter,hr);

	hr = ZDKDisplay_CreateTexture(480,272,&game_texture);
	if(FAILED(hr))
		printf("CreateTexture failed: %08x\n",scaleFlipCounter,hr);

	VGAScreen = VGAScreenSeg = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	VGAScreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	game_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	
	scale_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	dispSprite.Color = 0xFFFFFFFF;

	dispSprite.Vertices[0].X = 36;
	dispSprite.Vertices[0].Y = 120;
	dispSprite.Vertices[0].U = 0;
	dispSprite.Vertices[0].V = 1;
	
	dispSprite.Vertices[1].X = 235;
	dispSprite.Vertices[1].Y = 120;
	dispSprite.Vertices[1].U = 0;
	dispSprite.Vertices[1].V = 0;
	
	dispSprite.Vertices[2].X = 235;
	dispSprite.Vertices[2].Y = 440;
	dispSprite.Vertices[2].U = 1;
	dispSprite.Vertices[2].V = 0;
	
	dispSprite.Vertices[3].X = 36;
	dispSprite.Vertices[3].Y = 440;
	dispSprite.Vertices[3].U = 1;
	dispSprite.Vertices[3].V = 1;

	SDL_FillRect(VGAScreen, NULL, 0);

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
	SDL_FreeSurface(scale_surface);
	
	HRESULT hr = ZDKDisplay_FreeTexture(zdkDisplay);
	if(FAILED(hr))
		printf("FreeTexture failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);
	hr = ZDKDisplay_FreeTexture(layout_texture);
	if(FAILED(hr))
		printf("FreeTexture failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);
	hr = ZDKDisplay_FreeTexture(game_texture);
	if(FAILED(hr))
		printf("FreeTexture failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);
	ZDKDisplay_Cleanup();
}

void JE_clr256( SDL_Surface * screen)
{
	memset(screen->pixels, 0, screen->pitch * screen->h);
}

void JE_showVGA( void )
{
	scale_and_flip(VGAScreen);
}

void scale_and_flip( SDL_Surface *src_surface )
{
	if(!keyboardOpen)
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

		hr = ZDKDisplay_BeginScene();
		if(FAILED(hr))
			printf("BeginScene failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);

		hr = ZDKDisplay_SetTextureData(zdkDisplay, &dispRect, scale_surface->pixels, 4*320*200);
		if(FAILED(hr))
			printf("SetTextureData failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);

		draw_layout();

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
}

void load_texture( int button, WCHAR *name, bool menu )
{
	HIMAGE image;
	HRESULT hr;
	DWORD width, height;
	hr = ZDKImage_CreateImageFromFile(name, &image);
	if(FAILED(hr))
	{
		wprintf(L"CreatImage failed on %s: %08x\n", name, hr);
		return;
	}
	ZDKImage_GetImageSize(image, &width, &height);
	void *buffer = malloc(width*height*4);
	hr = ZDKImage_GetImageData(image, buffer, width*height*4);
	if(FAILED(hr))
	{
		wprintf(L"GetImageData for %s failed: %08x\n", name, hr);
	}
	if(button > 6)
	{
		hr = ZDKDisplay_CreateTexture(width, height, &layout.texture);
		if(FAILED(hr))
		{
			printf("CreatTexture failed on layout: %08x\n", hr);
			return;
		}
		hr = ZDKDisplay_SetTextureData(layout.texture, &layout.rect, buffer, width*height*4);
		if(FAILED(hr))
		{
			printf("SetTextureData failed on layout: %08x\n", hr);
			return;
		}
	}
	else
	{
		if(menu)
		{
			hr = ZDKDisplay_CreateTexture(width, height, &layout.buttons[button].menuTex);
			if(FAILED(hr))
			{
				printf("Creat Menu Texture failed on %s: %08x\n", layout.buttons[button].name, hr);
				return;
			}
			hr = ZDKDisplay_SetTextureData(layout.buttons[button].menuTex, &layout.buttons[button].rect, buffer, width*height*4);
			if(FAILED(hr))
			{
				printf("Set Menu Texture Data failed on %s: %08x\n", layout.buttons[button].name, hr);
				return;
			}
		}
		else
		{
			hr = ZDKDisplay_CreateTexture(width, height, &layout.buttons[button].gameTex);
			if(FAILED(hr))
			{
				printf("Creat Game Texture failed on %s: %08x\n", layout.buttons[button].name, hr);
				return;
			}
			hr = ZDKDisplay_SetTextureData(layout.buttons[button].gameTex, &layout.buttons[button].rect, buffer, width*height*4);
			if(FAILED(hr))
			{
				printf("Set Game Texture Data failed on %s: %08x\n", layout.buttons[button].name, hr);
				return;
			}
		}
	}
	free(buffer);
	ZDKImage_ReleaseImage(image);
}

/*void draw_layout( void )
{
	HRESULT hr;
	hr = ZDKDisplay_SetTexture(zdkDisplay);
	if(FAILED(hr))
		printf("SetTexture failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);

	hr = ZDKDisplay_DrawSprites(&dispSprite, 1);
	if(FAILED(hr))
		printf("DrawSprites failed: Try %d.  Error: %08x\n",scaleFlipCounter,hr);
}*/

// kate: tab-width 4; vim: set noet:
