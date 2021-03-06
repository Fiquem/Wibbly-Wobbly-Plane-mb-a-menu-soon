#pragma once

#include "text.h"

typedef void (*MenuFunction)();

typedef struct MenuOption{
	Text text;
	// alright, funciton pointers, leggo
	MenuFunction function_pointer;
	void function() { this->function_pointer(); };
}MenuOption;

typedef struct Menu{
	MenuOption* options;
	int length;
	float width, height, xpos, ypos;
	int selected;
}Menu;

Menu create_menu(Font f, const char** opts, MenuFunction* funcs, int l, float w, float h, float x, float y);
void draw_menu(Menu m);
void increment_menu_selected(Menu* m);
void decrement_menu_selected(Menu* m);
void select_menu_item(Menu* m);