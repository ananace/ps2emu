#include "ps2dev.h"
#include "ps2input.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>

#include <dirent.h>
#include <libgen.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 8

#define BUF_STATE 0

enum _ps2input_buf_kbd
{
    BUF_KBD_LED = 1,
    BUF_KBD_RATE,
    BUF_KBD_DELAY
};

enum _ps2input_buf_mouse
{
    BUF_MOUSE_X = 1,
    BUF_MOUSE_Y,
    BUF_MOUSE_B,
};

#define STATE_MODE    ((1 << 0) | \
                       (1 << 1))
#define STATE_DISABLED (1 << 2)

enum _ps2input_mode_mouse
{
    MODE_STREAM,
    MODE_ECHO,
    MODE_REMOTE
};

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

        case 0xF3:
            ps2dev_write(dev, PS2_ACK);
            ps2dev_read(dev, &data);
            ps2dev_write(dev, PS2_ACK);

            {
                float repeat_rate  = (pow(2, (data & 0x18) >> 3) * (8 + (data & 0x3)) / 240.f);
                uint16_t repeat_delay = 250 + ((data & 0x60) >> 5) * 250;

                inp->buf[BUF_KBD_RATE] = repeat_rate;
                inp->buf[BUF_KBD_DELAY] = repeat_delay;

#ifdef DEBUG
                printf("Requesting input repeat at %.1f/s after %dms\n", repeat_rate, repeat_delay);
#endif
            }
            return 0;

        case 0xF5:
            inp->buf[BUF_STATE] &= ~STATE_DISABLED;
        case 0xF6:
            ps2dev_write(dev, PS2_ACK);

            // Reset all options
            ps2input_set_led(inp, LED_SCROLLL, 0);
            ps2input_set_led(inp, LED_NUML, 0);
            ps2input_set_led(inp, LED_CAPSL, 0);
            return 0;
        }
    }
    else
    {
        if ((inp->buf[BUF_STATE] & STATE_MODE) == MODE_ECHO)
        {
            if (cmd == 0xEC)
            {
                ps2dev_write(dev, PS2_ACK);
                inp->buf[BUF_STATE] &= ~STATE_MODE;
                return 0;
            }
            else if (cmd == 0xFF)
            {
                ps2dev_write(dev, PS2_ACK);
                memset(inp->buf, 0, PS2INPUT_BUF);
                return 0;
            }

            ps2dev_write(dev, cmd);
            return 0;
        }

        switch (cmd)
        {

        case 0xE6:
            ps2dev_write(dev, PS2_ACK);
            ps2dev_read(dev, &data);
            ps2dev_write(dev, PS2_ACK);

#ifdef DEBUG
            printf("Requesting scaling at %dx\n", data);
#endif
            return 0;

        case 0xE9:
            ps2dev_write(dev, PS2_NAK);

            /*
             * ps2dev_write(dev, status);
             * ps2dev_write(dev, resolution);
             * ps2dev_write(dev, sample_rate);
             */

            return 0;

        case 0xEA:
        case 0xEC:
            ps2dev_write(dev, PS2_ACK);

            inp->buf[BUF_STATE] &= ~STATE_MODE;

#ifdef DEBUG
            printf("Setting mode: stream.\n");
#endif
            return 0;

        case 0xEB:
            ps2dev_write(dev, PS2_ACK);
            ps2dev_mouse_write(inp->target_device, inp->buf[BUF_MOUSE_X], inp->buf[BUF_MOUSE_Y], inp->buf[BUF_MOUSE_B]);
            return 0;

        case 0xED:
            ps2dev_write(dev, PS2_ACK);
            ps2dev_read(dev, &data);
            ps2dev_write(dev, PS2_ACK);

            {
                uint8_t counts = pow(2, data);
                (void)counts;

#ifdef DEBUG
                printf("Requesting resolution at %d count(s)/mm\n", counts);
#endif
            }
            return 0;

        case 0xEE:
            ps2dev_write(dev, PS2_ACK);

            inp->buf[BUF_STATE] &= ~STATE_MODE;
            inp->buf[BUF_STATE] |= MODE_ECHO;

#ifdef DEBUG
            printf("Setting mode: echo/wrap.\n");
#endif
            return 0;

        case 0xF0:
            ps2dev_write(dev, PS2_ACK);

            inp->buf[BUF_STATE] &= ~STATE_MODE;
            inp->buf[BUF_STATE] |= MODE_REMOTE;

#ifdef DEBUG
            printf("Setting mode: remote.\n");
#endif
            return 0;

        case 0xF3:
            ps2dev_write(dev, PS2_ACK);
            ps2dev_read(dev, &data);
            ps2dev_write(dev, PS2_ACK);

#ifdef DEBUG
            printf("Requesting sample rate as %d\n", data);
#endif
            return 0;

        case 0xF6:
            // Reset all options
            inp->buf[BUF_STATE] &= ~STATE_MODE;

        case 0xF5:
            inp->buf[BUF_STATE] &= ~STATE_DISABLED;

            ps2dev_write(dev, PS2_ACK);
            return 0;
        }
    }

    switch (cmd)
    {
    case 0xF4: // Enable streaming
        ps2dev_write(dev, PS2_ACK);
        inp->buf[BUF_STATE] &= ~STATE_DISABLED;
        return 0;

    case 0xFF:
        ps2dev_write(dev, PS2_ACK);

        memset(inp->buf, 0, PS2INPUT_BUF);

        ps2dev_write(dev, 0xAA);
        while (ps2dev_write(dev, 0x00) != 0)
            usleep(40);
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
                if ((input->buf[BUF_STATE] & STATE_MODE) == MODE_STREAM &&
                   (input->buf[BUF_STATE] & STATE_DISABLED) == 0)
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

            if ((input->buf[BUF_STATE] & STATE_MODE) == MODE_STREAM &&
               (input->buf[BUF_STATE] & STATE_DISABLED) == 0)
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
            if (obj.code == SYN_REPORT && input->target_device->type == PS2DEV_MOUSE &&
               (input->buf[BUF_STATE] & STATE_MODE) == MODE_STREAM &&
               (input->buf[BUF_STATE] & STATE_DISABLED) == 0)
                ps2dev_mouse_write(input->target_device, input->buf[BUF_MOUSE_X], input->buf[BUF_MOUSE_Y], input->buf[BUF_MOUSE_B]);
            break;
        }
    }

    return 0;
}

// TODO: Better method?
FILE* _ps2input_get_ledfile(struct ps2input* input, int led, const char* mode)
{
    const char* ledname;
    switch (led)
    {
    case LED_CAPSL: ledname = "capslock"; break;
    case LED_NUML: ledname = "numlock"; break;
    case LED_SCROLLL: ledname = "scrolllock"; break;

    default:
        return 0;
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
        return 0;

    sprintf(buf, "/sys/class/input/%s/device/%s/brightness", input->event, input_led);
    free(input_led);

    return fopen(buf, mode);
}

int ps2input_get_led(struct ps2input* input, int led)
{
    FILE* brightness = _ps2input_get_ledfile(input, led, "r");
    if (brightness == 0)
        return -1;

    char buf[32];
    fread(buf, 32, 1, brightness);
    fclose(brightness);

    return atoi(buf);
}

int ps2input_set_led(struct ps2input* input, int led, int value)
{
    FILE* brightness = _ps2input_get_ledfile(input, led, "w");
    if (brightness == 0)
        return -1;

    fwrite(value ? "1" : "0", 1, 1, brightness);
    fclose(brightness);

    return 0;
}
