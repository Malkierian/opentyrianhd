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
#include "network.h"
#include "nortvars.h"
#include "opentyr.h"
#include "mainint.h"
#include "setup.h"
#include "video.h"

#include "SDL.h"

void JE_textMenuWait( JE_word *waitTime, JE_boolean doGamma )
{
	do
	{
		JE_showVGA();
		
		inputFound = update_input();
		
		if (doGamma)
			JE_gammaCheck();
		
		NETWORK_KEEP_ALIVE();
		
		SDL_Delay(16);
		
		if (*waitTime > 0)
		{
			(*waitTime)--;
		}
	} while (!(inputFound || *waitTime == 1));
}

// kate: tab-width 4; vim: set noet:
