#include <zdkinput.h>

#include "input.h"
#include "opentyr.h"
#include "nortsong.h"
#include "varz.h"
#include "video.h"

#include <zdkext.h>
#include <zdkinput.h>
#include <zdkdisplay.h>

input_struct softPad;

bool inputFound;

bool buttons[7];

bool buttons_in_menu;

float analog_max = 60.0f;

layout_struct layout;

WCHAR layoutPath[42] = /*L"\\Flash2\\Tyrian\\Layout";*/L"\\gametitle\\584E07D1\\Content\\Tyrian\\Layout";

void pos_from_input(int *x, int *y, bool in_menu, int min_menu, int max_menu, bool play_sound)
{
	if(in_menu)
	{
		if(x != NULL && (abs(softPad.ax) > abs(softPad.ay)))
		{
			if (softPad.ax > 0)
			{
				stickX += softPad.ax;
				if(stickX > 30)
				{
					stickX = 0;
					if(*x == max_menu - 1)
						*x = 0;
					else
						*x++;
					if(play_sound)
						JE_playSampleNum(S_CURSOR);
				}
			}
			else if (softPad.ax < 0 && x != NULL)
			{
				stickX += softPad.ax;
				if(stickX < -30)
				{
					stickX = 0;
					if(*x == 0)
						*x = max_menu - 1;
					else
						*x--;
					if(play_sound)
						JE_playSampleNum(S_CURSOR);
				}
			}
		}
		else if(y != NULL && abs(softPad.ay) > abs(softPad.ax))
		{
			if (softPad.ay > 0 && y != NULL)
			{
				stickY += softPad.ay;
				if(stickY > 30)
				{
					stickY = 0;
					*y += 1;
					if(*y > max_menu - 1)
						*y = min_menu;
					if(play_sound)
						JE_playSampleNum(S_CURSOR);
				}
			}
			else if (softPad.ay < 0  && y != NULL)
			{
				stickY +=softPad.ay;
				if(stickY < -30)
				{
					stickY = 0;
					*y -= 1;
					if(*y < min_menu)
						*y = max_menu - 1;
					if(play_sound)
						JE_playSampleNum(S_CURSOR);
				}
			}
		}
	}
}

bool is_button_pressed( layout_button_struct button, ZDK_TOUCH_LOCATION location )
{
	if(button.sprite.Vertices[0].X < location.X && location.X < button.sprite.Vertices[1].X)
		if(button.sprite.Vertices[0].Y < location.Y && location.Y < button.sprite.Vertices[2].Y)
			return true;
	return false;
}

bool is_trackpad_pressed( layout_trackpad_struct trackpad, ZDK_TOUCH_LOCATION location )
{
	if(trackpad.sprite.Vertices[0].X < location.X && location.X < trackpad.sprite.Vertices[1].X)
	{
		if(trackpad.sprite.Vertices[0].Y < location.Y && location.Y < trackpad.sprite.Vertices[2].Y)
		{
			trackpad.origin.x = location.X;
			trackpad.origin.y = location.Y;
			trackpad.location = location;
			return true;
		}
	}
	return false;
}

bool update_input( void )
{
	if(ZDKExt_ExitRequested())
	{
		JE_tyrianHalt(0);
	}
	ZDK_INPUT_STATE input;
	ZDK_TOUCH_STATE touch;
	ZDK_TOUCH_LOCATION curLoc;
	ZDK_SPRITE curSprite;

	ZDKInput_GetState(&input);
	touch = input.TouchState;
	int touchCount = touch.Count;
	int padZones = 0, joyZone = 0, button = 0;
	bool pressed[7] = {false,false,false,false,false}; //mode, escape, select, lkick, rkick, menu, keyboard

	softPad.button_pressed = false;
	softPad.direction_pressed = false;

	if(touchCount == 0)
	{
		if(softPad.mode)
			softPad.mode = false;
		else
			softPad.mode_last = false;
		
		if(softPad.escape)
			softPad.escape = false;
		else
			softPad.escape_last = false;
		
		if(softPad.select)
			softPad.select = false;
		else
			softPad.select_last = false;
		
		if(softPad.lKick)
			softPad.lKick = false;
		else
			softPad.lKick_last = false;
		
		if(softPad.rKick)
			softPad.rKick = false;
		else
			softPad.rKick_last = false;
		
		if(softPad.menu)
			softPad.menu = false;
		else
			softPad.menu_last = false;
		
		if(softPad.key)
			softPad.key = false;
		else
			softPad.key_last = false;

		softPad.tracking = false;

		return false;
	}
	padZones = 5;

	for(int i = 0; i < touchCount; i++)
	{
		if(is_trackpad_pressed(layout.trackpad, touch.Locations[i]))
		{
			if(softPad.tracking)
			{
				softPad.ax = touch.Locations[i].Y - softPad.trackpad_last_loc.Y; // this is where the conversion from Zune's x,y to
				softPad.ay = softPad.trackpad_last_loc.X - touch.Locations[i].X; // OT's x,y is made.
				//softPad.direction_pressed = true;
			}
			else
			{
				softPad.tracking = true;
				softPad.ax = 0.0f;
				softPad.ay = 0.0f;
			}
			softPad.direction_pressed = true;
			softPad.trackpad_last_loc = touch.Locations[i];
			continue;
		}
		if(is_button_pressed(layout.buttons[0], touch.Locations[i])) // mode
		{
			softPad.button_pressed = true;
			if(softPad.mode)
				softPad.mode_last = true;
			else
				softPad.mode = true;
			pressed[0] = true;
		}
		if(is_button_pressed(layout.buttons[1], touch.Locations[i])) // escape
		{
			softPad.button_pressed = true;
			if(softPad.escape)
				softPad.escape_last = true;
			else
				softPad.escape = true;
			pressed[1] = true;
		}
		if(is_button_pressed(layout.buttons[2], touch.Locations[i])) // select
		{
			softPad.button_pressed = true;
			if(softPad.select)
				softPad.select_last = true;
			else
				softPad.select = true;
			pressed[2] = true;
		}
		if(is_button_pressed(layout.buttons[3], touch.Locations[i])) // lKick
		{
			softPad.button_pressed = true;
			if(softPad.lKick)
				softPad.lKick_last = true;
			else
				softPad.lKick = true;
			pressed[3] = true;
		}
		if(is_button_pressed(layout.buttons[4], touch.Locations[i])) // rKick
		{
			softPad.button_pressed = true;
			if(softPad.rKick)
				softPad.rKick_last = true;
			else
				softPad.rKick = true;
			pressed[4] = true;
		}
		if(is_button_pressed(layout.buttons[5], touch.Locations[i])) // menu
		{
			softPad.button_pressed = true;
			if(softPad.menu)
				softPad.menu_last = true;
			else
				softPad.menu = true;
			pressed[5] = true;
		}
		if(is_button_pressed(layout.buttons[6], touch.Locations[i])) // keyboard
		{
			softPad.button_pressed = true;
			if(softPad.key)
				softPad.key_last = true;
			else
				softPad.key = true;
			pressed[6] = true;
		}
	}
	if(!pressed[0])
	{
		if(softPad.mode)
			softPad.mode = false;
		else
			softPad.mode_last = false;
	}
	if(!pressed[1])
	{
		if(softPad.escape)
			softPad.escape = false;
		else
			softPad.escape_last = false;
	}
	if(!pressed[2])
	{
		if(softPad.select)
			softPad.select = false;
		else
			softPad.select_last = false;
	}
	if(!pressed[3])
	{
		if(softPad.lKick)
			softPad.lKick = false;
		else
			softPad.lKick_last = false;
	}
	if(!pressed[4])
	{
		if(softPad.rKick)
			softPad.rKick = false;
		else
			softPad.rKick_last = false;
	}
	if(!pressed[5])
	{
		if(softPad.menu)
			softPad.menu = false;
		else
			softPad.menu_last = false;
	}
	if(!pressed[6])
	{
		if(softPad.key)
			softPad.key = false;
		else
			softPad.key_last = false;
	}
	if(!softPad.direction_pressed)
		softPad.tracking = false;
	if(!(softPad.button_pressed || softPad.direction_pressed))
		return false;
	else
		return true;
}

void set_rect (int top, int bottom, int left, int right, ZDK_RECT *rect)
{
	rect->Bottom = bottom;
	rect->Left = left;
	rect->Right = right;
	rect->Top = top;
}

void set_sprite (int top, int bottom, int left, int right, ZDK_SPRITE *sprite)
{
	sprite->Color = 0xFFFFFFFF;
	
	sprite->Vertices[0].X = left;
	sprite->Vertices[0].Y = top;
	sprite->Vertices[0].U = 0;
	sprite->Vertices[0].V = 1;
	
	sprite->Vertices[1].X = right;
	sprite->Vertices[1].Y = top;
	sprite->Vertices[1].U = 0;
	sprite->Vertices[1].V = 0;

	sprite->Vertices[2].X = right;
	sprite->Vertices[2].Y = bottom;
	sprite->Vertices[2].U = 1;
	sprite->Vertices[2].V = 0;
	
	sprite->Vertices[3].X = left;
	sprite->Vertices[3].Y = bottom;
	sprite->Vertices[3].U = 1;
	sprite->Vertices[3].V = 1;
}

void deinit_layout( void )
{
	HRESULT hr;
	for(int i = 0; i < 7; i++)
	{
		if(layout.buttons[i].gameTex != NULL)
		{
			hr = ZDKDisplay_FreeTexture(layout.buttons[i].gameTex);
			if(FAILED(hr))
				printf("FreeTexture failed: %08x\n",hr);
		}
		if(layout.buttons[i].menuTex != NULL)
		{
			hr = ZDKDisplay_FreeTexture(layout.buttons[i].menuTex);
			if(FAILED(hr))
				printf("FreeTexture failed: %08x\n",hr);
		}
	}
	hr = ZDKDisplay_FreeTexture(layout.texture);
	if(FAILED(hr))
		printf("FreeTexture failed: %08x\n",hr);
}

void init_layout( void )
{
	WCHAR path[56];

	for(int i = 0; i < 7; i++)
	{
		buttons[i] = false;
	}
	buttons_in_menu = true;

	strcpy(layout.buttons[PAD_BUTTON_MODE].name, "mode");
	strcpy(layout.buttons[PAD_BUTTON_ESCAPE].name, "escape");
	strcpy(layout.buttons[PAD_BUTTON_SELECT].name, "select");
	strcpy(layout.buttons[PAD_BUTTON_LKICK].name, "lKick");
	strcpy(layout.buttons[PAD_BUTTON_RKICK].name, "rKick");
	strcpy(layout.buttons[PAD_BUTTON_MENU].name, "menu");
	strcpy(layout.buttons[PAD_BUTTON_KEY].name, "keyboard");

	for(int i = 0; i < 5; i++) // Set rect for buttons 0-4, since they're the same
	{
		set_rect(0, 90, 0, 90, &layout.buttons[i].rect);
	}
	set_rect(0, 40, 0, 90, &layout.buttons[PAD_BUTTON_MENU].rect);
	set_rect(0, 40, 0, 90, &layout.buttons[PAD_BUTTON_KEY].rect);
	set_rect(0, 272, 0, 480, &layout.rect);
	
	set_sprite(299, 388, 182, 271, &layout.buttons[PAD_BUTTON_MODE].sprite);
	set_sprite(299, 388, 0, 89, &layout.buttons[PAD_BUTTON_ESCAPE].sprite);
	set_sprite(390, 479, 91, 180, &layout.buttons[PAD_BUTTON_SELECT].sprite);
	set_sprite(390, 479, 182, 271, &layout.buttons[PAD_BUTTON_LKICK].sprite);
	set_sprite(390, 479, 0, 89, &layout.buttons[PAD_BUTTON_RKICK].sprite);
	set_sprite(208, 297, 232, 271, &layout.buttons[PAD_BUTTON_MENU].sprite);
	set_sprite(208, 297, 0, 39, &layout.buttons[PAD_BUTTON_KEY].sprite);

	swprintf(path, L"%s\\%s", layoutPath, L"mode.png");
	load_texture(PAD_BUTTON_MODE, path, false);
	load_texture(PAD_BUTTON_MODE, path, true);

	swprintf(path, L"%s\\%s", layoutPath, L"pause.png");
	load_texture(PAD_BUTTON_ESCAPE, path, false);
	swprintf(path, L"%s\\%s", layoutPath, L"cancel.png");
	load_texture(PAD_BUTTON_ESCAPE, path, true);

	swprintf(path, L"%s\\%s", layoutPath, L"fire.png");
	load_texture(PAD_BUTTON_SELECT, path, false);
	swprintf(path, L"%s\\%s", layoutPath, L"select.png");
	load_texture(PAD_BUTTON_SELECT, path, true);

	swprintf(path, L"%s\\%s", layoutPath, L"lKick.png");
	load_texture(PAD_BUTTON_LKICK, path, false);
	swprintf(path, L"%s\\%s", layoutPath, L"pageUp.png");
	load_texture(PAD_BUTTON_LKICK, path, true);

	swprintf(path, L"%s\\%s", layoutPath, L"rKick.png");
	load_texture(PAD_BUTTON_RKICK, path, false);
	swprintf(path, L"%s\\%s", layoutPath, L"pageDn.png");
	load_texture(PAD_BUTTON_RKICK, path, true);

	swprintf(path, L"%s\\%s", layoutPath, L"menu.png");
	load_texture(PAD_BUTTON_MENU, path, false);
	layout.buttons[PAD_BUTTON_MENU].menuTex = NULL;

	swprintf(path, L"%s\\%s", layoutPath, L"keyboard.png");
	load_texture(PAD_BUTTON_KEY, path, false);
	load_texture(PAD_BUTTON_KEY, path, true);

	swprintf(path, L"%s\\%s", layoutPath, L"layout.png");
	load_texture(PAD_LAYOUT, path, false);
	set_sprite(0, 479, 0, 271, &layout.sprite);

	set_sprite(0,207,36,247,&layout.trackpad.sprite);
}

void init_input( void )
{
	ZDKInput_Initialize();
	init_layout();

	inputFound = false;

	softPad.ax = 0.0f;
	softPad.ay = 0.0f;
	softPad.trackpad_last_loc.X = 0.0f,
		softPad.trackpad_last_loc.Y = 0.0f,
		softPad.trackpad_last_loc.Pressure = 0.0f;
	softPad.tracking = false;

	softPad.direction_pressed = false;
	softPad.button_pressed = false;

	softPad.up_last = false;
	softPad.down_last = false;
	softPad.left_last = false;
	softPad.right_last = false;
	softPad.select = false;
	softPad.select_last = false;
	softPad.escape = false;
	softPad.escape_last = false;
	softPad.lKick = false;
	softPad.lKick_last = false;
	softPad.rKick = false;
	softPad.rKick_last = false;
	softPad.menu = false;
	softPad.menu_last = false;
	softPad.mode = false;
	softPad.mode_last = false;
	softPad.key = false;
	softPad.key_last = false;
}

void deinit_input( void )
{
	ZDKInput_Shutdown();
}

void draw_layout( void )
{
	HRESULT hr;
	hr = ZDKDisplay_SetTexture(layout.texture);
	if(FAILED(hr))
		printf("SetTexture failed: %08x\n", hr);

	hr = ZDKDisplay_DrawSprites(&layout.sprite, 1);
	if(FAILED(hr))
		printf("DrawSprites failed: %08x\n", hr);

	for(int i = 0; i < 7; i++)
	{
		if(buttons[i] && buttons_in_menu)
		{
			hr = ZDKDisplay_SetTexture(layout.buttons[i].menuTex);
			if(FAILED(hr))
				printf("SetTexture failed: %08x\n",hr);
			ZDKDisplay_DrawSprites(&layout.buttons[i].sprite, 1);
			if(FAILED(hr))
				printf("DrawSprites failed: %08x\n", hr);
		}
		else if(buttons[i] && !buttons_in_menu)
		{
			hr = ZDKDisplay_SetTexture(layout.buttons[i].gameTex);
			if(FAILED(hr))
				printf("SetTexture failed: %08x\n",hr);
			ZDKDisplay_DrawSprites(&layout.buttons[i].sprite, 1);
			if(FAILED(hr))
				printf("DrawSprites failed: %08x\n", hr);
		}
	}
}

void set_layout_buttons( bool mode, bool escape, bool select, bool lKick, bool rKick, bool menu, bool key)
{
	buttons[0] = mode;
	buttons[1] = escape;
	buttons[2] = select;
	buttons[3] = lKick;
	buttons[4] = rKick;
	buttons[5] = menu;
	buttons[6] = key;
}