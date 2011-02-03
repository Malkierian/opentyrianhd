#ifndef INPUT_H
#define INPUT_H

#define DPAD_ZONES 13
#define JOY_ZONES 6

#define PAD_BUTTON_MODE		0
#define PAD_BUTTON_ESCAPE	1
#define PAD_BUTTON_SELECT	2
#define PAD_BUTTON_LKICK	3
#define PAD_BUTTON_RKICK	4
#define PAD_BUTTON_MENU		5
#define PAD_BUTTON_KEY		6
#define PAD_LAYOUT			7

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
	bool menu, menu_last;
	bool key, key_last;
};

struct coord_struct {
	float x;
	float y;
};

struct layout_trackpad_struct {
	ZDK_SPRITE sprite;
	ZDK_RECT rect;
	ZDK_TOUCH_LOCATION location;
	coord_struct origin;
};

struct layout_button_struct {
	HTEXTURE menuTex;
	HTEXTURE gameTex;
	ZDK_SPRITE sprite;
	ZDK_RECT rect;
	char name[9];
};

struct layout_struct {
	layout_button_struct buttons[7]; // mode, cancel/pause, select/fire, lKick/pgUp, rKick/pgDn, [none]/menu, keyboard
	layout_trackpad_struct trackpad;
	
	HTEXTURE texture;
	ZDK_SPRITE sprite;
	ZDK_RECT rect;
};

bool update_input( void );
void pos_from_input( int *x, int *y, bool in_menu, int min_menu, int max_menu, bool play_sound);
void init_input( void );
void deinit_input( void );
void init_layout( void );
void deinit_layout( void );
void draw_layout( void );
void set_layout_buttons( bool mode, bool escape, bool select, bool lKick, bool rKick, bool menu, bool key);

extern layout_struct layout;
extern input_struct softPad;
extern bool inputFound;
extern bool buttons[7];
extern bool buttons_in_menu;

#endif