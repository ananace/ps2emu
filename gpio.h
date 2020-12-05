#ifndef _GPIO_H
#define _GPIO_H

#include <stdint.h>

#define GPIO_LOW 0
#define GPIO_HIGH 1

#define GPIO_IN 0
#define GPIO_OUT 1
#define GPIO_INOUT 2

int gpio_init(uint8_t dev);
int gpio_uninit();

int gpio_export(int pin, int dir);
int gpio_unexport(int fd);
int gpio_write(int fd, int value);
int gpio_read(int fd);

#endif
