#include "ps2dev.h"
#include <linux/input-event-codes.h>

int main(int argc, char** argv)
{
    struct ps2dev dev;

    ps2dev_init(&dev, PS2DEV_KEYBOARD, 1, 2);

    ps2dev_keyboard_press(&dev, KEY_LEFT);
    ps2dev_keyboard_release(&dev, KEY_LEFT);
    ps2dev_keyboard_press(&dev, KEY_RIGHT);
    ps2dev_keyboard_release(&dev, KEY_RIGHT);

    ps2dev_keyboard_press(&dev, KEY_SYSRQ);
    ps2dev_keyboard_release(&dev, KEY_SYSRQ);
    ps2dev_keyboard_press(&dev, KEY_PAUSE);
    ps2dev_keyboard_release(&dev, KEY_PAUSE);

    return 0;
}
