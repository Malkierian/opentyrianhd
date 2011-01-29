#include <zdkinput.h>

#include "input.h"
#include "opentyr.h"
#include "nortsong.h"
#include "varz.h"

#include <zdkext.h>
#include <zdkinput.h>
#include <zdkdisplay.h>

input_struct softPad;

bool inputFound;

float analog_max = 60.0f;

layout_struct layout;

void pos_from_input(int *x, int *y, bool in_menu, int min_menu, int max_menu, bool play_sound)
{
	if(in_menu)
	{
		if(x != NULL && abs(softPad.ax) > abs(softPad.ay))
		{
			if (softPad.ax > 0)
			{
				stickX += softPad.ax;
				if(stickX > 40)
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
				if(stickX < -40)
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
				if(stickY > 40)
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
				if(stickY < -40)
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
	else
	{
		*x += (softPad.ax/analog_max) * 5.4f;
		*y -= (softPad.ay/analog_max) * 5.4f;
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
	bool pressed[5] = {false,false,false,false,false}; //mode, escape, select, lkick, rkick

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
				softPad.direction_pressed = true;
			}
			else
			{
				softPad.tracking = true;
				softPad.ax = 0.0f;
				softPad.ay = 0.0f;
			}
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
			continue;
		}
		if(is_button_pressed(layout.buttons[1], touch.Locations[i])) // escape
		{
			softPad.button_pressed = true;
			if(softPad.escape)
				softPad.escape_last = true;
			else
				softPad.escape = true;
			pressed[1] = true;
			continue;
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
	if(!(softPad.button_pressed || softPad.direction_pressed))
		return false;
	else
		return true;
}

/*void set_rect (int top, int bottom, int left, int right, ZDK_RECT *rect)
{
	rect->Bottom = bottom;
	rect->Left = left;
	rect->Right = right;
	rect->Top = top;
}*/

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

void init_layout( void )
{
	strcpy(layout.buttons[0].name, "mode");
	layout.buttons[0].name[4] = 0;
	set_sprite(419,449,150,180,&layout.buttons[0].sprite);

	strcpy(layout.buttons[1].name, "escape");
	layout.buttons[1].name[6] = 0;
	set_sprite(419,449,121,151,&layout.buttons[1].sprite);

	strcpy(layout.buttons[2].name, "select");
	layout.buttons[2].name[6] = 0;
	set_sprite(449,479,121,151,&layout.buttons[2].sprite);

	strcpy(layout.buttons[3].name, "lKick");
	layout.buttons[3].name[5] = 0;
	set_sprite(449,479,91,121,&layout.buttons[3].sprite);

	strcpy(layout.buttons[4].name, "rKick");
	layout.buttons[4].name[5] = 0;
	set_sprite(449,479,151,181,&layout.buttons[4].sprite);

	//layout.joystick.center.x = 136.0f;
	//layout.joystick.center.y = 60.0f;
	set_sprite(0,120,76,196,&layout.trackpad.sprite);
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
	softPad.mode = false;
	softPad.mode_last = false;
}

void deinit_input( void )
{
	ZDKInput_Shutdown();
}