#ifndef _PS2INPUT_H
#define _PS2INPUT_H

#include <stdint.h>
#include <stdio.h>

#define PS2INPUT_BUF 8

struct ps2input
{
    const char* event;

    FILE* source_device;
    struct ps2dev* target_device;

    int buf[PS2INPUT_BUF];
};

int ps2input_init(struct ps2input* input, const char* source, struct ps2dev* target);
int ps2input_deinit(struct ps2input* input);

int ps2input_poll(struct ps2input* input);
int ps2input_get_led(struct ps2input* input, int led);
int ps2input_set_led(struct ps2input* input, int led, int value);

#endif
