/* 
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2010  The OpenTyrian Development Team
 * 
 * Scale2x, Scale3x
 * Copyright (C) 2001, 2002, 2003, 2004 Andrea Mazzoleni
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

#include "video_scale.h"

#include "palette.h"
#include "video.h"

#include <assert.h>

static void no_scale( SDL_Surface *src_surface, SDL_Surface *dst_surface );
static void nn_32( SDL_Surface *src_surface, SDL_Surface *dst_surface );
static void nn_16( SDL_Surface *src_surface, SDL_Surface *dst_surface );

uint scaler;

const struct Scalers scalers[] =
{
	{ 1 * vga_width, 1 * vga_height, no_scale, nn_16,      nn_32,      "None" }
};
const uint scalers_count = COUNTOF(scalers);

void set_scaler_by_name( const char *name )
{
	for (uint i = 0; i < scalers_count; ++i)
	{
		if (strcmp(name, scalers[i].name) == 0)
		{
			scaler = i;
			break;
		}
	}
}

void no_scale( SDL_Surface *src_surface, SDL_Surface *dst_surface )
{
	Uint8 *src = (Uint8 *)src_surface->pixels,
	      *dst = (Uint8 *)dst_surface->pixels;
	
	memcpy(dst, src, src_surface->pitch * src_surface->h);
}


void nn_32( SDL_Surface *src_surface, SDL_Surface *dst_surface )
{
	Uint8 *src = (Uint8 *)src_surface->pixels, *src_temp,
	      *dst = (Uint8 *)dst_surface->pixels, *dst_temp;
	int src_pitch = src_surface->pitch,
	    dst_pitch = dst_surface->pitch;
	const int dst_Bpp = 4;         // dst_surface->format->BytesPerPixel
	
	const int height = vga_height, // src_surface->h
	          width = vga_width,   // src_surface->w
	          scale = dst_surface->w / width;
	assert(scale == dst_surface->h / height);
	
	for (int y = height; y > 0; y--)
	{
		src_temp = src;
		dst_temp = dst;
		
		for (int x = width; x > 0; x--)
		{
			for (int z = scale; z > 0; z--)
			{
				*(Uint32 *)dst = rgb_palette[*src];
				dst += dst_Bpp;
			}
			src++;
		}
		
		src = src_temp + src_pitch;
		dst = dst_temp + dst_pitch;
		
		for (int z = scale; z > 1; z--)
		{
			memcpy(dst, dst_temp, dst_pitch);
			dst += dst_pitch;
		}
	}
}

void nn_16( SDL_Surface *src_surface, SDL_Surface *dst_surface )
{
	Uint8 *src = (Uint8 *)src_surface->pixels, *src_temp,
	      *dst = (Uint8 *)dst_surface->pixels, *dst_temp;
	int src_pitch = src_surface->pitch,
	    dst_pitch = dst_surface->pitch;
	const int dst_Bpp = 2;         // dst_surface->format->BytesPerPixel
	
	const int height = vga_height, // src_surface->h
	          width = vga_width,   // src_surface->w
	          scale = dst_surface->w / width;
	assert(scale == dst_surface->h / height);
	
	for (int y = height; y > 0; y--)
	{
		src_temp = src;
		dst_temp = dst;
		
		for (int x = width; x > 0; x--)
		{
			for (int z = scale; z > 0; z--)
			{
				*(Uint16 *)dst = rgb_palette[*src];
				dst += dst_Bpp;
			}
			src++;
		}
		
		src = src_temp + src_pitch;
		dst = dst_temp + dst_pitch;
		
		for (int z = scale; z > 1; z--)
		{
			memcpy(dst, dst_temp, dst_pitch);
			dst += dst_pitch;
		}
	}
}

// kate: tab-width 4; vim: set noet:
