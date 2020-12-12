#include "ps2dev.h"
#include "ps2input.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>

#include <dirent.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 8

#define BUF_STATE 0
#define BUF_MOUSE_X 1
#define BUF_MOUSE_Y 2
#define BUF_MOUSE_B 3
#define BUF_KBD_LED 3

#define STATE_STREAM (1 << 0)

int _ps2dev_handle_cmd(struct ps2dev* dev, uint8_t cmd)
{
    struct ps2input* inp = (struct ps2input*)dev->user_data;
    uint8_t data;

    if (dev->type == PS2DEV_KEYBOARD)
    {
        switch (cmd)
        {
        case 0xED:
            ps2dev_write(dev, PS2_ACK);
            ps2dev_read(dev, &data);
            ps2dev_write(dev, PS2_ACK);

            ps2input_set_led(inp, LED_SCROLLL, (data & 0x01) == 0x01);
            ps2input_set_led(inp, LED_NUML, (data & 0x02) == 0x02);
            ps2input_set_led(inp, LED_CAPSL, (data & 0x04) == 0x04);
            return 0;
        }
    }
    else
    {
    }

    switch (cmd)
    {
    case 0xF4: // Enable streaming
        ps2dev_write(dev, PS2_ACK);
        inp->buf[BUF_STATE] |= STATE_STREAM;
        return 0;

    case 0xF5: // Disable streaming
        ps2dev_write(dev, PS2_ACK);
        inp->buf[BUF_STATE] &= ~STATE_STREAM;
        return 0;
    }

    return 1;
}

int ps2input_init(struct ps2input* input, const char* source, struct ps2dev* target)
{
    memset(input, 0, sizeof(struct ps2input));

    input->source_device = fopen(source, "rb");
    input->target_device = target;
    char* buf = strdup(source);
    input->event = strdup(basename(buf));
    free(buf);

    input->target_device->handle_command = _ps2dev_handle_cmd;
    input->target_device->user_data = (long)input;

    return 0;
}
int ps2input_deinit(struct ps2input* input)
{
    if (input->source_device != 0)
        fclose(input->source_device);
    if (input->event != 0)
        free((void*)input->event);

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
                if (input->buf[BUF_STATE] & STATE_STREAM)
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
            }
            else
            {
                uint8_t bit = 255;
                if (obj.code == BTN_MIDDLE || obj.code == BTN_TOOL_DOUBLETAP)
                    bit = 0;
                else if (obj.code == BTN_RIGHT || obj.code == BTN_TOOL_TRIPLETAP)
                    bit = 1;
                else if (obj.code == BTN_LEFT) // TODO: Tapping/doubletap-and-drag? (obj.code == BTN_TOUCH)
                    bit = 2;

                if (bit != 255)
                {
                    if (obj.value == 1)
                        input->buf[BUF_MOUSE_B] |= 1 << bit;
                    else
                        input->buf[BUF_MOUSE_B] &= ~(1 << bit);
                }
            }
            break;

        case EV_LED:
            {
                if (obj.value == 1)
                    input->buf[BUF_KBD_LED] |= 1 << obj.code;
                else
                    input->buf[BUF_KBD_LED] &= ~(1 << obj.code);
            }

            if (input->buf[BUF_STATE] & STATE_STREAM)
            {
                ps2dev_write(input->target_device, 0xED);
                ps2dev_write(input->target_device, (uint8_t)(input->buf[BUF_KBD_LED] & 0xFF));
            }
            break;

        case EV_REL:
            if (obj.code == REL_X)
                input->buf[BUF_MOUSE_X] = obj.value;
            else if (obj.code == REL_Y)
                input->buf[BUF_MOUSE_Y] = obj.value;
            break;

        case EV_SYN:
            // TODO: Rate
            if (obj.code == SYN_REPORT && input->target_device->type == PS2DEV_MOUSE
                && (input->buf[BUF_STATE] & STATE_STREAM))
                ps2dev_mouse_write(input->target_device, input->buf[BUF_MOUSE_X], input->buf[BUF_MOUSE_Y], input->buf[BUF_MOUSE_B]);
            break;
        }
    }

    return 0;
}

// TODO: Better method?
int ps2input_set_led(struct ps2input* input, int led, int value)
{
    const char* ledname;
    switch (led)
    {
    case LED_CAPSL: ledname = "capslock"; break;
    case LED_NUML: ledname = "numlock"; break;
    case LED_SCROLLL: ledname = "scrolllock"; break;

    default:
        return -1;
    }

    char buf[256];
    sprintf(buf, "/sys/class/input/%s/device/", input->event);
    DIR* directory = opendir(buf);
    struct dirent* entry;

    char* input_led = 0;
    while ((entry = readdir(directory)))
    {
        if (strstr(entry->d_name, ledname))
        {
            input_led = strdup(entry->d_name);
            break;
        }
    }
    closedir(directory);

    if (input_led == 0)
        return -2;

    sprintf(buf, "/sys/class/input/%s/device/%s/brightness", input->event, input_led);
    free(input_led);

    FILE* brightness = fopen(buf, "w");
    fwrite(value ? "1" : "0", 1, 1, brightness);
    fclose(brightness);

    return 0;
}
