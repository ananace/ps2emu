#ifndef _PS2DEV_H
#define _PS2DEV_H

#include <stdint.h>

#define PS2_ERR1       0x00
#define PS2_TEST_SUCC  0xAA
#define PS2_ECHO       0xEE
#define PS2_ACK        0xFA
#define PS2_TEST_FAIL1 0xFC
#define PS2_NAK2       0xFC
#define PS2_TEST_FAIL2 0xFD
#define PS2_RESEND     0xFE
#define PS2_NAK        0xFE
#define PS2_ERR2       0xFF

#define PS2DEV_KEYBOARD 0
#define PS2DEV_MOUSE 1

struct ps2dev
{
    int type;
#ifdef DEBUG
    const char* name;
#endif

    int clkfd, datafd;
    uint8_t last_byte;

    long user_data;
    int(*handle_command)(struct ps2dev*, uint8_t);
};

int ps2dev_init(struct ps2dev* dev, int type, int clk, int data);
int ps2dev_deinit(struct ps2dev* dev);
int ps2dev_poll(struct ps2dev* dev);

int ps2dev_available(struct ps2dev* dev);
int ps2dev_write(struct ps2dev* dev, uint8_t data);
int ps2dev_read(struct ps2dev* dev, uint8_t* data);

int ps2dev_keyboard_press(struct ps2dev* dev, int key);
int ps2dev_keyboard_release(struct ps2dev* dev, int key);

int ps2dev_mouse_write(struct ps2dev* dev, int x, int y, int buttons);
// int ps2dev_mouse_write_mw(struct ps2dev* dev, int x, int y, int z, int buttons);

#endif
