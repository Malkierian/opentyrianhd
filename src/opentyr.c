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
#include "config.h"
#include "destruct.h"
#include "editship.h"
#include "episodes.h"
#include "file.h"
#include "font.h"
#include "helptext.h"
#include "hg_revision.h"
#include "input.h"
#include "joystick.h"
#include "jukebox.h"
#include "keyboard.h"
#include "loudness.h"
#include "mainint.h"
#include "mtrand.h"
#include "musmast.h"
#include "network.h"
#include "nortsong.h"
#include "opentyr.h"
#include "params.h"
#include "picload.h"
#include "scroller.h"
#include "setup.h"
#include "sprite.h"
#include "tyrian2.h"
#include "xmas.h"
#include "varz.h"
#include "vga256d.h"
#include "video.h"
#include "video_scale.h"

#include "SDL.h"

#include <altcecrt.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <windows.h>
#include <zdksystem.h>

const char *opentyrian_str = "OpenTyrian",
           *opentyrian_version = "Classic (" HG_REV ")";
const char *opentyrian_menu_items[] =
{
	"About OpenTyrian",
	/* "Play Destruct", */
	"Jukebox",
	"Rich Mode: Off",
	"Return to Main Menu"
};

/* zero-terminated strncpy */
char *strnztcpy( char *to, const char *from, size_t count )
{
	to[count] = '\0';
	return strncpy(to, from, count);
}

void SuppressReboot()
{
  HKEY key = NULL;
  HRESULT hr = S_OK;
  DWORD value;

  if (SUCCEEDED(hr))
    hr = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\Power\\State\\Reboot", 0, 0, &key));
  if (SUCCEEDED(hr))
    hr = HRESULT_FROM_WIN32(RegSetValueEx(key, L"Flags", 0, REG_DWORD, (BYTE *)&(value = 0x10000), sizeof(DWORD)));
  if (SUCCEEDED(hr))
    hr = HRESULT_FROM_WIN32(RegSetValueEx(key, L"Default", 0, REG_DWORD, (BYTE *)&(value = 0), sizeof(DWORD)));
  if (key)
    RegCloseKey(key);
}

void opentyrian_menu( void )
{
	set_layout_buttons(0, 1, 1, 0, 0, 0, 0);
	int sel = 0;
	const int maxSel = COUNTOF(opentyrian_menu_items) - 1;
	bool quit = false, fade_in = true;
	
	uint temp_scaler = scaler;
	
	fade_black(10);
	JE_loadPic(VGAScreen, 13, false);

	draw_font_hv(VGAScreen, VGAScreen->w / 2, 5, opentyrian_str, large_font, centered, 15, -3);

	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

	JE_showVGA();

	play_song(36); // A Field for Mag

	do
	{
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);

		for (int i = 0; i <= maxSel; i++)
		{
			const char *text = opentyrian_menu_items[i];
			char buffer[100];

			if (i == 2) /* Scaler */
			{
				sprintf(buffer, "Rich Mode: %s", richMode ? "On" : "Off");
				text = buffer;
			}

			draw_font_hv_shadow(VGAScreen, VGAScreen->w / 2, (i != maxSel) ? i * 16 + 32 : 118, text, normal_font, centered, 15, (i != sel) ? -4 : -2, false, 2);
		}

		JE_showVGA();

		if (fade_in)
		{
			fade_in = false;
			fade_palette(colors, 20, 0, 255);
		}

		tempW = 0;
		JE_textMenuWait(&tempW, false);

		if (inputFound)
		{
			if(softPad.button_pressed)
			{
				if(softPad.select && !softPad.select_last)
				{
					switch (sel)
					{
						case 0: /* About */
							JE_playSampleNum(S_SELECT);

							scroller_sine(about_text);

							memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
							JE_showVGA();
							fade_in = true;
							break;
						case 1: /* Jukebox */
							JE_playSampleNum(S_SELECT);

							fade_black(10);
							jukebox();

							memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
							JE_showVGA();
							fade_in = true;
							break;
						case 2:
							richMode ^= 1;
							break;
						default: /* Return to main menu */
							quit = true;
							JE_playSampleNum(S_SPRING);
							break;
					}
				}
				if(softPad.escape && !softPad.escape_last)
				{
					quit = true;
					JE_playSampleNum(S_SPRING);
				}
			}
			else if(softPad.direction_pressed)
				pos_from_input(NULL, &sel, true, 0, maxSel + 1, true);
		}
	} while (!quit);
}

int main( int argc, char *argv[] )
{
	//mt_srand((unsigned int)_time64(NULL));  // What does this do?

	printf("\nWelcome to... >> %s %s <<\n\n", opentyrian_str, opentyrian_version);

	printf("Copyright (C) 2007-2009 The OpenTyrian Development Team\n\n");

	printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	printf("This is free software, and you are welcome to redistribute it\n");
	printf("under certain conditions.  See the file GPL.txt for details.\n\n");

	if (SDL_Init(0))
	{
		printf("Failed to initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	CreateDirectory(L"\\Flash2\\Tyrian", NULL);
	CreateDirectory(L"\\Flash2\\Tyrian\\userdata", NULL);

	JE_loadConfiguration();

	xmas = xmas_time();  // arg handler may override

	JE_paramCheck(argc, argv);

	JE_scanForEpisodes();

	init_video();
	init_keyboard();
	init_joysticks();
	init_input();
	ZDKSystem_SetOrientation((ZDK_ORIENTATION)2);
	SuppressReboot();
	printf("assuming mouse detected\n"); // SDL can't tell us if there isn't one

	if (xmas && (!dir_file_exists(data_dir(), "tyrianc.shp") || !dir_file_exists(data_dir(), "voicesc.snd")))
	{
		xmas = false;

		fprintf(stderr, "warning: Christmas is missing.\n");
	}

	JE_loadPals();
	JE_loadMainShapeTables(xmas ? "tyrianc.shp" : "tyrian.shp");

	if (xmas && !xmas_prompt())
	{
		xmas = false;

		free_main_shape_tables();
		JE_loadMainShapeTables("tyrian.shp");
	}


	/* Default Options */
	youAreCheating = false;
	smoothScroll = true;
	loadDestruct = false;

	if (!audio_disabled)
	{
		printf("initializing SDL audio...\n");

		init_audio();

		load_music();

		JE_loadSndFile("tyrian.snd", xmas ? "voicesc.snd" : "voices.snd");
	}
	else
	{
		printf("audio disabled\n");
	}

	if (record_demo)
		printf("demo recording enabled (input limited to keyboard)\n");

	JE_loadExtraShapes();  /*Editship*/

	JE_loadHelpText();
	/*debuginfo("Help text complete");*/

	if (isNetworkGame)
	{
		if (network_init())
		{
			network_tyrian_halt(3, false);
		}
	}
	set_layout_buttons(0, 0, 1, 0, 0, 0, 0);

#ifdef NDEBUG
	if (!isNetworkGame)
		intro_logos();
#endif
	for (; ; )
	{
		JE_initPlayerData();
		JE_sortHighScores();
		
		if (JE_titleScreen(true))
			break;  // user quit from title screen

		if (loadDestruct)
		{
			JE_destructGame();
			loadDestruct = false;
		}
		else
		{
			JE_main();
		}
	}

	JE_tyrianHalt(0);

	return 0;
}

// kate: tab-width 4; vim: set noet:
