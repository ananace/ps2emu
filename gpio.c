#include "gpio.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <linux/gpio.h>
#include <sys/ioctl.h>

int g_gpio_fd = -1;

int gpio_init(uint8_t dev)
{
	if (g_gpio_fd >= 0)
		return 0;

	char chrdev_name[20];
	// TODO: Does this need to be configurable?
	snprintf(chrdev_name, 20, "/dev/gpiochip%i", dev);
	int fd = open(chrdev_name, 0);
	if (fd == -1) {
		int ret = -errno;
		fprintf(stderr, "Failed to open %s: %s (%d)\n", chrdev_name, strerror(-ret), ret);

		return ret;
	}

	g_gpio_fd = fd;

	struct gpiochip_info info;
	int ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info);
	if (ret < 0)
	{
		ret = -errno;
		fprintf(stderr, "Failed to read GPIO chip info: %s (%d)\n", strerror(-ret), ret);
		return ret;
	}

	printf("Attached to GPIO device /dev/%s (%s) with %d lines\n", info.name, info.label, info.lines);

	return 0;
}

int gpio_uninit()
{
	if (g_gpio_fd < 0)
		return 0;

	if (close(g_gpio_fd) < 0)
	{
		int ret = -errno;
		fprintf(stderr, "Failed to close GPIO device: %s (%d)\n", strerror(-ret), ret);

		return ret;
	}

	g_gpio_fd = -1;

	return 0;
}

int gpio_export(int pin, int dir)
{
	if (g_gpio_fd < 0)
		return -1;

	struct gpiohandle_request req;
	req.lineoffsets[0] = pin;
	req.flags = 0;
	if (dir == GPIO_IN || dir == GPIO_INOUT)
	    req.flags |= GPIOHANDLE_REQUEST_INPUT;
	if (dir == GPIO_OUT || dir == GPIO_INOUT)
	    req.flags |= GPIOHANDLE_REQUEST_OUTPUT;
	req.lines = 1;
	snprintf(req.consumer_label, 32, "ps2emu_gpio%d", pin);

	int ret = ioctl(g_gpio_fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
	if (ret < 0)
	{
		ret = -errno;
		fprintf(stderr, "Failed to export GPIO pin %d (%s): %s (%d)\n", pin, (dir == GPIO_OUT ? "OUT" : (dir == GPIO_IN ? "IN" : "INOUT")), strerror(-ret), ret);
		return ret;
	}

	return 0;
}

int gpio_unexport(int fd)
{
	if (fd <= 0)
		return 0;

	if (close(fd) < 0)
	{
		int ret = -errno;
		fprintf(stderr, "Failed to close GPIO pin %d: %s (%d)\n", fd, strerror(-ret), ret);
		return ret;
	}

	return 0;
}

int gpio_write(int fd, int value)
{
	if (g_gpio_fd <= 0 || fd <= 0)
		return -1;

	struct gpiohandle_data data;
	data.values[0] = value;

	int ret = ioctl(fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
	if (ret < 0)
	{
		ret = -errno;
		fprintf(stderr, "Failed to write GPIO value %d to pin %d: %s (%d)\n", value, fd, strerror(-ret), ret);
		return ret;
	}

	return 0;
}

int gpio_read(int fd)
{
	if (g_gpio_fd <= 0 || fd <= 0)
		return -1;

	struct gpiohandle_data data;
	data.values[0] = 0;

	int ret = ioctl(fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
	if (ret < 0)
	{
		ret = -errno;
		fprintf(stderr, "Failed to read GPIO value from pin %d: %s (%d)\n", fd, strerror(-ret), ret);
		return ret;
	}

	return data.values[0];
}
