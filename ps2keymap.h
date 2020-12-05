#ifndef _PS2KEYMAP_H
#define _PS2KEYMAP_H

#include <stdint.h>

int ps2keymap_has_key(int key);
int ps2keymap_get_press(int key, int size, void* out);
int ps2keymap_get_break(int key, int size, void* out);

#endif
