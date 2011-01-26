#include <zdkinput.h>

#include "input.h"
#include "opentyr.h"
#include "nortsong.h"
#include "varz.h"

#include <zdkinput.h>
#include <zdkdisplay.h>

input_struct softPad;

bool inputFound;

float analog_max = 60.0f;

layout_struct layout;

void pos_from_input(int *x, int *y, bool in_menu, int min_menu, int max_menu)
{
	if(in_menu)
	{
		if ((softPad.ax > 0 || softPad.dx > 0) && x != NULL)
		{
			stickX += softPad.analog ? softPad.ax : softPad.dx;
			if(stickX > 300)
			{
				stickX = 0;
				if(*x == max_menu - 1)
					*x = 0;
				else
					*x++;
				JE_playSampleNum(S_CURSOR);
			}
		}
		else if ((softPad.ax < 0 || softPad.dx < 0) && x != NULL)
		{
			stickX += softPad.analog ? softPad.ax : softPad.dx;
			if(stickX < -300)
			{
				stickX = 0;
				if(*x == 0)
					*x = max_menu - 1;
				else
					*x--;
				JE_playSampleNum(S_CURSOR);
			}
		}
		else if ((softPad.ay > 0 || softPad.dy > 0) && y != NULL)
		{
			stickY += softPad.analog ? softPad.ay : softPad.dy;
			if(stickY > 300)
			{
				stickY = 0;
				*y += 1;
				if(*y > max_menu - 1)
					*y = min_menu;
				JE_playSampleNum(S_CURSOR);
			}
		}
		else if ((softPad.ay < 0 || softPad.dy < 0 ) && y != NULL)
		{
			stickY += softPad.analog ? softPad.ay : softPad.dy;
			if(stickY < -300)
			{
				stickY = 0;
				*y -= 1;
				if(*y < min_menu)
					*y = max_menu - 1;
				JE_playSampleNum(S_CURSOR);
			}
		}
	}
	else
	{
		if(softPad.analog)
		{
			*x += (softPad.ax/analog_max) * 5.4f;
			*y -= (softPad.ay/analog_max) * 5.4f;
		}
		else
		{
			*x += softPad.dx * 5;
			*y -= softPad.dy * 5;
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

bool is_joypad_pressed( layout_joystick_struct joystick, ZDK_TOUCH_LOCATION location )
{
	if(joystick.sprite.Vertices[0].X < location.X && location.X < joystick.sprite.Vertices[1].X)
		if(joystick.sprite.Vertices[0].Y < location.Y && location.Y < joystick.sprite.Vertices[2].Y)
			return true;
	return false;
}

bool update_input( void )
{
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
		return false;
	padZones = 5;

	for(int i = 0; i < touchCount; i++)
	{
		if(is_joypad_pressed(layout.joystick, touch.Locations[i]))
		{
			softPad.ax = touch.Locations[i].Y - layout.joystick.center.y; // this is where the conversion from Zune's x,y to
			softPad.ay = layout.joystick.center.x - touch.Locations[i].X; // OT's x,y is made.
			if(softPad.ax < 10.0f && softPad.ax > -10.0f)
			{
				softPad.ax = 0.0f;
				continue;
			}
			if(softPad.ay < 10.0f && softPad.ay > -10.0f)
			{
				softPad.ay = 0.0f;
				continue;
			}
			softPad.direction_pressed = true;
			continue;
		}
		if(is_button_pressed(layout.buttons[0], touch.Locations[i])) // mode
		{
			softPad.button_pressed = true;
			if(softPad.mode)
				softPad.last_mode = true;
			else
				softPad.mode = true;
			pressed[0] = true;
			continue;
		}
		if(is_button_pressed(layout.buttons[1], touch.Locations[i])) // escape
		{
			softPad.button_pressed = true;
			if(softPad.escape)
				softPad.last_escape = true;
			else
				softPad.escape = true;
			pressed[1] = true;
			continue;
		}
		if(is_button_pressed(layout.buttons[2], touch.Locations[i])) // select
		{
			softPad.button_pressed = true;
			if(softPad.select)
				softPad.last_select = true;
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
			softPad.last_mode = false;
	}
	if(!pressed[1])
	{
		if(softPad.escape)
			softPad.escape = false;
		else
			softPad.last_escape = false;
	}
	if(!pressed[2])
	{
		if(softPad.select)
			softPad.select = false;
		else
			softPad.last_select = false;
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

	layout.joystick.center.x = 136.0f;
	layout.joystick.center.y = 60.0f;
	set_sprite(0,120,76,196,&layout.joystick.sprite);
}

void init_input( void )
{
	ZDKInput_Initialize();
	init_layout();

	inputFound = false;

	softPad.ax = 0.0f;
	softPad.ay = 0.0f;
	softPad.dx = 0;
	softPad.dy = 0;
	softPad.analog = true;

	softPad.direction_pressed = false;
	softPad.button_pressed = false;

	softPad.last_up = false;
	softPad.last_down = false;
	softPad.last_left = false;
	softPad.last_right = false;
	softPad.select = false;
	softPad.last_select = false;
	softPad.escape = false;
	softPad.last_escape = false;
	softPad.lKick = false;
	softPad.last_lKick = false;
	softPad.rKick = false;
	softPad.last_rKick = false;
	softPad.mode = false;
	softPad.last_mode = false;
}

void deinit_input( void )
{
	ZDKInput_Shutdown();
}