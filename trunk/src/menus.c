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
#include "episodes.h"
#include "fonthand.h"
#include "input.h"
#include "keyboard.h"
#include "menus.h"
#include "nortsong.h"
#include "opentyr.h"
#include "palette.h"
#include "picload.h"
#include "setup.h"
#include "sprite.h"
#include "video.h"

#include <zdksystem.h>

char episode_name[6][31], difficulty_name[7][21], gameplay_name[5][26];

bool select_gameplay( void )
{
	set_layout_buttons(0, 1, 1, 0, 0, 0, 0);
	JE_loadPic(VGAScreen, 2, false);
	JE_dString(VGAScreen, JE_fontCenter(gameplay_name[0], FONT_SHAPES), 20, gameplay_name[0], FONT_SHAPES);

	set_layout_buttons(0, 1, 1, 0, 0, 0, 0);

	int gameplay = 1,
	    gameplay_max = 2;

	bool fade_in = true;
	for (; ; )
	{
		for (int i = 1; i <= gameplay_max; i++)
		{
			JE_outTextAdjust(VGAScreen, JE_fontCenter(gameplay_name[i], SMALL_FONT_SHAPES), i * 24 + 30, gameplay_name[i], 15, - 4 + (i == gameplay ? 2 : 0) - (i == 4 ? 4 : 0), SMALL_FONT_SHAPES, true);
		}
		JE_showVGA();

		if (fade_in)
		{
			fade_palette(colors, 10, 0, 255);
			fade_in = false;
		}

		JE_word temp = 0;
		JE_textMenuWait(&temp, false);

		if (inputFound)
		{
			if(softPad.button_pressed)
			{
				if(softPad.select && !softPad.select_last)
				{
					JE_playSampleNum(S_SELECT);
					fade_black(10);
					onePlayerAction = (gameplay == 2);
					return true;
				}
				if(softPad.escape && !softPad.escape_last)
				{
					JE_playSampleNum(S_SPRING);
					return false;
				}
			}
			else if(softPad.direction_pressed)
			{
				pos_from_input(NULL, &gameplay, true, 1, gameplay_max + 1, true);
			}
		}
	}
}

bool select_episode( void )
{
	set_layout_buttons(0, 1, 1, 0, 0, 0, 0);
	JE_loadPic(VGAScreen, 2, false);
	JE_dString(VGAScreen, JE_fontCenter(episode_name[0], FONT_SHAPES), 20, episode_name[0], FONT_SHAPES);

	set_layout_buttons(0, 1, 1, 0, 0, 0, 0);

	int episode = 1,
	    episode_max = EPISODE_MAX - 1;

	bool fade_in = true;
	for (; ; )
	{
		for (int i = 1; i <= episode_max; i++)
		{
			JE_outTextAdjust(VGAScreen, 20, i * 30 + 20, episode_name[i], 15, -4 + (i == episode ? 2 : 0) - (!episodeAvail[i - 1] ? 4 : 0), SMALL_FONT_SHAPES, true);
		}
		JE_showVGA();

		if (fade_in)
		{
			fade_palette(colors, 10, 0, 255);
			fade_in = false;
		}

		JE_word temp = 0;
		JE_textMenuWait(&temp, false);

		if (inputFound)
		{
			if(softPad.button_pressed)
			{
				if(softPad.select && !softPad.select_last)
				{
					if (!episodeAvail[episode - 1])
					{
						JE_playSampleNum(S_SPRING);
						break;
					}
					JE_playSampleNum(S_SELECT);
					fade_black(10);

					JE_initEpisode(episode);
					initial_episode_num = episodeNum;
					return true;
				}
				if(softPad.escape && !softPad.escape_last)
				{
					JE_playSampleNum(S_SPRING);
					return false;
				}
			}
			else if(softPad.direction_pressed)
			{
				pos_from_input(NULL, &episode, true, 1, episode_max + 1, true);
			}
		}
	}
}

bool select_difficulty( void )
{
	set_layout_buttons(0, 1, 1, 0, 0, 0, 1);
	JE_loadPic(VGAScreen, 2, false);
	JE_dString(VGAScreen, JE_fontCenter(difficulty_name[0], FONT_SHAPES), 20, difficulty_name[0], FONT_SHAPES);

	set_layout_buttons(0, 1, 1, 0, 0, 0, 1);

	WCHAR keyTemp[9] = L"";
	DWORD keyboardState;
	bool codeEntered = false;

	difficultyLevel = 2;
	int difficulty_max = 3;

	bool fade_in = true;
	for (; ; )
	{
		for (int i = 1; i <= difficulty_max; i++)
		{
			JE_outTextAdjust(VGAScreen, JE_fontCenter(difficulty_name[i], SMALL_FONT_SHAPES), i * 24 + 30, difficulty_name[i], 15, -4 + (i == difficultyLevel ? 2 : 0), SMALL_FONT_SHAPES, true);
		}
		JE_showVGA();

		if (fade_in)
		{
			fade_palette(colors, 10, 0, 255);
			fade_in = false;
		}

		JE_word temp = 0;
		JE_textMenuWait(&temp, false);
		
		if (inputFound)
		{
			if(softPad.button_pressed)
			{
				if(softPad.select && !softPad.select_last)
				{
					JE_playSampleNum(S_SELECT);

					if (difficultyLevel == 6)
					{
						difficultyLevel = 8;
					} else if (difficultyLevel == 5) {
						difficultyLevel = 6;
					}
					return true;
				}
				if(softPad.escape && !softPad.escape_last)
				{
					JE_playSampleNum(S_SPRING);
					return false;
				}
				if(softPad.key && !softPad.key_last)
				{
					bool quitKey = false;
					ZDKSystem_ShowKeyboard(L"", L"Close", NULL);
					while(!quitKey)
					{
						keyboardState = ZDKSystem_GetKeyboardState();
						if(keyboardState == KEYBOARD_STATE_DISMISSED)
						{
							quitKey = true;
							codeEntered = true;
							size_t size;
							ZDKSystem_GetKeyboardBufferText(keyTemp, 8, &size);
							JE_playSampleNum(S_SELECT);
						}
						if(keyboardState == KEYBOARD_STATE_CLOSED)
						{
							quitKey = true;
						}
						SDL_Delay(250);
					}
					ZDKSystem_CloseKeyboard();
					if(codeEntered)
					{
						codeEntered = false;
						int length = wcslen(keyTemp);
						if(length == 1)
						{
							if(difficulty_max < 4 && _wcsicmp(keyTemp, L"G") == 0)
								difficulty_max++;
							else if(difficulty_max == 4 && wcscmp(keyTemp, L"]") == 0)
								difficulty_max++;
						}
						else if(length == 4)
						{
							if(difficulty_max == 5)
								if(_wcsicmp(keyTemp, L"LORD") == 0)
									difficulty_max++;
						}
					}
				}
			}
			else if(softPad.direction_pressed)
			{
				pos_from_input(NULL, &difficultyLevel, true, 1, difficulty_max + 1, true);
			}
		}
	}
}

// kate: tab-width 4; vim: set noet:
