#include "ps2dev.h"
#include "ps2input.h"
#include <linux/input-event-codes.h>

int main(int argc, char** argv)
{
    struct ps2dev dev;
    ps2dev_init(&dev, PS2DEV_KEYBOARD, 1, 2);

    struct ps2input inp;
    ps2input_init(&inp, "/dev/input/event4", &dev);

    ps2input_set_led(&inp, LED_CAPSL, 1);
    ps2input_set_led(&inp, LED_CAPSL, 0);

    return 0;


#ifdef DEBUG
    dev->name = "Testing device";
#endif

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
