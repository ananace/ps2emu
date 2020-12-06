#ifndef _PS2INPUT_H
#define _PS2INPUT_H

#include <stdint.h>
#include <stdio.h>

struct ps2input
{
    FILE* source_device;
    struct ps2dev* target_device;

    int buf[16];

    int(*handle_command)(uint8_t);
};

int ps2input_init(struct ps2input* input, const char* source, struct ps2dev* target);
int ps2input_deinit(struct ps2input* input);

int ps2input_poll(struct ps2input* input);

#endif
