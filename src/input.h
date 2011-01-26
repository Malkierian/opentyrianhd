#ifndef INPUT_H
#define INPUT_H

#define DPAD_ZONES 13
#define JOY_ZONES 6

#define PAD_BUTTON_MODE		0
#define PAD_BUTTON_ESCAPE	1
#define PAD_BUTTON_SELECT	2
#define PAD_BUTTON_LKICK	3
#define PAD_BUTTON_RKICK	4

#define PAD_JOY 5

#define PAD_DPAD 5

#define PAD_DPAD_UP		6
#define PAD_DPAD_DOWN	7
#define PAD_DPAD_LEFT	8
#define PAD_DPAD_RIGHT	9

#include <zdkinput.h>
#include <zdkdisplay.h>

struct input_struct {
	float ax;
	float ay;
	signed int dx;
	signed int dy;
	bool analog;

	bool direction_pressed;
	bool button_pressed;

	bool last_up, last_down, last_left, last_right;
	bool select, last_select;
	bool escape, last_escape;
	bool lKick, last_lKick;
	bool rKick, last_rKick;
	bool mode, last_mode;
};

struct coord_struct {
	float x;
	float y;
};

struct layout_joystick_struct {
	HTEXTURE unpressed;
	HTEXTURE pressed;
	ZDK_SPRITE sprite;
	ZDK_RECT hitArea;
	coord_struct center;
};

struct layout_dpad_struct {
	HTEXTURE unpressed;
	HTEXTURE pressed;
	ZDK_SPRITE sprite;
	ZDK_RECT hitArea;
	int direction;
};

struct layout_button_struct {
	HTEXTURE unpressed;
	HTEXTURE pressed;
	ZDK_SPRITE sprite;
	ZDK_RECT hitArea;
	char name[8];
};

struct layout_struct {
	layout_button_struct buttons[5];
	layout_dpad_struct dpad[4];
	layout_joystick_struct joystick;
};

bool update_input( void );
void pos_from_input( int *x, int *y, bool in_menu, int min_menu, int max_menu);
void init_input( void );
void deinit_input( void );
void load_layouts( void );

extern input_struct softPad;
extern bool inputFound;

#endif