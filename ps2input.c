#include "ps2dev.h"
#include "ps2input.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>

#include <string.h>

#define CHUNK_SIZE 8

#define BUF_MOUSE_X 0
#define BUF_MOUSE_Y 1
#define BUF_MOUSE_B 2
#define BUF_MOUSE_S 2
#define BUF_KBD_LED 3

#define MOUSE_STATE_STREAM 1

int ps2input_init(struct ps2input* input, const char* source, struct ps2dev* target)
{
    memset(input, 0, sizeof(struct ps2input));

    input->source_device = fopen(source, "rb");
    input->target_device = target;

    return 0;
}
int ps2input_deinit(struct ps2input* input)
{
    if (input->source_device != 0)
        fclose(input->source_device);

    input->source_device = 0;

    return 0;
}

int ps2input_poll(struct ps2input* input)
{
    if (input->source_device == 0)
        return -2;
    if (input->target_device == 0)
        return -3;

    struct input_event buf[CHUNK_SIZE];
    size_t r = fread(buf, sizeof(struct input_event), CHUNK_SIZE, input->source_device);

    for (size_t id = 0; id <= (r / sizeof(struct input_event)); ++id)
    {
        struct input_event obj = buf[id];
        switch (obj.type)
        {
        case EV_KEY:
            if (input->target_device->type == PS2DEV_KEYBOARD)
            {
                if (obj.value == 0)
                    ps2dev_keyboard_release(input->target_device, obj.code);
                else if (obj.value == 1)
                    ps2dev_keyboard_press(input->target_device, obj.code);
                else if (obj.value == 2)
                {
                    // TODO: Repeats
                }
            }
            else
            {
                uint8_t bit = 0;
                if (obj.code == BTN_MIDDLE)
                    bit = 0;
                else if (obj.code == BTN_RIGHT)
                    bit = 1;
                else if (obj.code == BTN_LEFT)
                    bit = 2;

                if (obj.value == 1)
                    input->buf[BUF_MOUSE_B] |= 1 << bit;
                else
                    input->buf[BUF_MOUSE_B] &= ~(1 << bit);
            }
            break;

        case EV_LED:
            {
                if (obj.value == 1)
                    input->buf[BUF_KBD_LED] |= 1 << obj.code;
                else
                    input->buf[BUF_KBD_LED] &= ~(1 << obj.code);
            }

            ps2dev_write(input->target_device, 0xED);
            ps2dev_write(input->target_device, (uint8_t)(input->buf[BUF_KBD_LED] & 0xFF));
            break;

        case EV_REL:
            if (obj.code == REL_X)
                input->buf[BUF_MOUSE_X] = obj.value;
            else if (obj.code == REL_Y)
                input->buf[BUF_MOUSE_Y] = obj.value;
            break;

        case EV_SYN:
            if (obj.code == SYN_REPORT && input->target_device->type == PS2DEV_MOUSE
                && input->buf[BUF_MOUSE_S] == MOUSE_STATE_STREAM)
                ps2dev_mouse_write(input->target_device, input->buf[BUF_MOUSE_X], input->buf[BUF_MOUSE_Y], input->buf[BUF_MOUSE_B]);
            break;
        }
    }

    return 0;
}
