#include "gpio.h"
#include "ps2dev.h"
#include "ps2keymap.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define PS2CLK_FULL 40
#define PS2CLK_HALF 20
#define PS2CLK_DELAY 400
#define PS2CLK_TIMEOUT 30

void _ps2dev_pulse(int fd)
{
    usleep(PS2CLK_HALF);
    gpio_write(fd, GPIO_LOW);
    usleep(PS2CLK_FULL);
    gpio_write(fd, GPIO_HIGH);
    usleep(PS2CLK_HALF);
}

int ps2dev_init(struct ps2dev* dev, int type, int clk, int data)
{
    dev->type = type;
    dev->clkfd = gpio_export(clk, GPIO_OUT);
    dev->datafd = gpio_export(data, GPIO_INOUT);

    return 0;
}
int ps2dev_deinit(struct ps2dev* dev)
{
    gpio_unexport(dev->clkfd);
    gpio_unexport(dev->datafd);

    return 0;
}

int ps2dev_available(struct ps2dev* dev)
{
    return (gpio_read(dev->datafd) == GPIO_LOW || gpio_read(dev->clkfd) == GPIO_LOW);
}

int ps2dev_write(struct ps2dev* dev, uint8_t data)
{
    usleep(PS2CLK_DELAY);

    gpio_write(dev->datafd, GPIO_LOW);
    _ps2dev_pulse(dev->clkfd);

    unsigned char parity = 1;
    for (int i = 0; i < 8; ++i)
    {
        if (data & 0x01)
            gpio_write(dev->datafd, GPIO_HIGH);
        else
            gpio_write(dev->datafd, GPIO_LOW);
        _ps2dev_pulse(dev->clkfd);

        parity = parity ^ (data & 0x01);
        data = data >> 1;
    }

    if (parity)
        gpio_write(dev->datafd, GPIO_HIGH);
    else
        gpio_write(dev->datafd, GPIO_LOW);
    _ps2dev_pulse(dev->clkfd);

    gpio_write(dev->datafd, GPIO_HIGH);
    _ps2dev_pulse(dev->clkfd);

    usleep(PS2CLK_DELAY);

#ifdef DEBUG
    printf("Wrote data: %x, parity: %i\n", data, parity);
#endif

    return 0;
}
int ps2dev_read(struct ps2dev* dev, uint8_t* value)
{
    // Wait for data
    clock_t start = clock();
    while (gpio_read(dev->datafd) != GPIO_LOW || gpio_read(dev->clkfd) != GPIO_HIGH)
    {
        if (((double)(clock() - start)) / CLOCKS_PER_SEC > (1.0 / PS2CLK_TIMEOUT))
            return -1;
    }

    int data = 0x00;
    uint32_t bit = 0x01;
    uint8_t calculated_parity = 1;
    uint8_t received_parity = 0;
    _ps2dev_pulse(dev->clkfd);

    while (bit < 0x0100) {
        if (gpio_read(dev->datafd) == GPIO_HIGH)
        {
            data = data | bit;
            calculated_parity = calculated_parity ^ 1;
        }
        else
            calculated_parity = calculated_parity ^ 0;

        bit = bit << 1;
        _ps2dev_pulse(dev->clkfd);
    }

    if (gpio_read(dev->datafd) == GPIO_HIGH)
        received_parity = 1;

    _ps2dev_pulse(dev->clkfd);

    usleep(PS2CLK_HALF);
    gpio_write(dev->datafd, GPIO_LOW);
    gpio_write(dev->clkfd, GPIO_LOW);
    usleep(PS2CLK_FULL);
    gpio_write(dev->clkfd, GPIO_HIGH);
    usleep(PS2CLK_HALF);
    gpio_write(dev->datafd, GPIO_HIGH);

#ifdef DEBUG
    printf("Received data: %x, parity: %i, calculated parity: %i\n", data & 0x00ff, calculated_parity, received_parity);
#endif

    *value = (uint8_t)(data & 0x00ff);
    if (calculated_parity != received_parity)
        return -2;

    return 0;
}

int ps2dev_keyboard_press(struct ps2dev* dev, int key)
{
    uint8_t press[8] = {};
    int bytes;
    if ((bytes = ps2keymap_get_press(key, 8, press)) < 0)
        return -1;

#ifdef DEBUG
    printf("Pressing key %d, resulting in %d bytes [%0x %0x %0x %0x %0x %0x %0x %0x]\n", key, bytes, press[0], press[1], press[2], press[3], press[4], press[5], press[6], press[7]);
#endif

    for (int i = 0; i < bytes; ++i)
        ps2dev_write(dev, press[i]);

    return 0;
}
int ps2dev_keyboard_release(struct ps2dev* dev, int key)
{
    uint8_t release[8] = {};
    int bytes;
    if ((bytes = ps2keymap_get_break(key, 8, release)) < 0)
        return -1;

#ifdef DEBUG
    printf("Releasing key %d, resulting in %d bytes [%x %x %x %x %x %x %x %x]\n", key, bytes, release[0], release[1], release[2], release[3], release[4], release[5], release[6], release[7]);
#endif

    for (int i = 0; i < bytes; ++i)
        ps2dev_write(dev, release[i]);

    return 0;
}

uint8_t _ps2dev_clip(uint32_t a)
{
    return (a & (~0xff)) ? ((-a)>>31) : (a);
}
int ps2dev_mouse_write(struct ps2dev* dev, int x, int y, int buttons)
{
    uint8_t byte = 0;
    byte |= (abs(x) > 255) << 7;
    byte |= (abs(y) > 255) << 6;
    byte |= (x < 0) << 5;
    byte |= (y < 0) << 4;
    byte |= 1 << 3;
    byte |= (buttons & 0x01) << 2;
    byte |= ((buttons & 0x02) >> 1) << 1;
    byte |= ((buttons & 0x04) >> 2) << 0;

    ps2dev_write(dev, byte);
    ps2dev_write(dev, _ps2dev_clip(abs(x)));
    ps2dev_write(dev, _ps2dev_clip(abs(y)));

    return 0;
}
