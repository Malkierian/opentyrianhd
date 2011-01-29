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
	ZDK_TOUCH_LOCATION trackpad_last_loc;
	bool tracking;

	bool direction_pressed;
	bool button_pressed;

	bool up_last, down_last, left_last, right_last;
	bool select, select_last;
	bool escape, escape_last;
	bool lKick, lKick_last;
	bool rKick, rKick_last;
	bool mode, mode_last;
};

struct coord_struct {
	float x;
	float y;
};

struct layout_trackpad_struct {
	ZDK_SPRITE sprite;
	ZDK_RECT hitArea;
	ZDK_TOUCH_LOCATION location;
	coord_struct origin;
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
	layout_trackpad_struct trackpad;
};

bool update_input( void );
void pos_from_input( int *x, int *y, bool in_menu, int min_menu, int max_menu, bool play_sound);
void init_input( void );
void deinit_input( void );
void load_layouts( void );

extern input_struct softPad;
extern bool inputFound;

#endif