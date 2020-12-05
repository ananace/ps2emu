#include "ps2keymap.h"
#include <linux/input-event-codes.h>

uint16_t ps2keymap_press[] = {
    /* KEY_RESERVED */   0x00,
    /* KEY_ESC */        0x76,
    /* KEY_1 */          0x16,
    /* KEY_2 */          0x1e,
    /* KEY_3 */          0x26,
    /* KEY_4 */          0x25,
    /* KEY_5 */          0x2e,
    /* KEY_6 */          0x36,
    /* KEY_7 */          0x3d,
    /* KEY_8 */          0x3e,
    /* KEY_9 */          0x46,
    /* KEY_0 */          0x45,
    /* KEY_MINUS */      0x4e,
    /* KEY_EQUAL */      0x55,
    /* KEY_BACKSPACE */  0x66,
    /* KEY_TAB */        0x0d,
    /* KEY_Q */          0x15,
    /* KEY_W */          0x1d,
    /* KEY_E */          0x24,
    /* KEY_R */          0x2d,
    /* KEY_T */          0x2c,
    /* KEY_Y */          0x35,
    /* KEY_U */          0x3c,
    /* KEY_I */          0x43,
    /* KEY_O */          0x44,
    /* KEY_P */          0x4d,
    /* KEY_LEFTBRACE */  0x54,
    /* KEY_RIGHTBRACE */ 0x5b,
    /* KEY_ENTER */      0x5a,
    /* KEY_LEFTCTRL */   0x14,
    /* KEY_A */          0x1c,
    /* KEY_S */          0x1b,
    /* KEY_D */          0x23,
    /* KEY_F */          0x2b,
    /* KEY_G */          0x34,
    /* KEY_H */          0x33,
    /* KEY_J */          0x3b,
    /* KEY_K */          0x42,
    /* KEY_L */          0x4b,
    /* KEY_SEMICOLON */  0x4c,
    /* KEY_APOSTROPHE */ 0x52,
    /* KEY_GRAVE */      0x0e,
    /* KEY_LEFTSHIFT */  0x12,
    /* KEY_BACKSLASH */  0x5d,
    /* KEY_Z */          0x1a,
    /* KEY_X */          0x22,
    /* KEY_C */          0x21,
    /* KEY_V */          0x2a,
    /* KEY_B */          0x32,
    /* KEY_N */          0x31,
    /* KEY_M */          0x3a,
    /* KEY_COMMA */      0x41,
    /* KEY_DOT */        0x49,
    /* KEY_SLASH */      0x4a,
    /* KEY_RIGHTSHIFT */ 0x59,
    /* KEY_KPASTERISK */ 0x7c,
    /* KEY_LEFTALT */    0x11,
    /* KEY_SPACE */      0x29,
    /* KEY_CAPSLOCK */   0x58,
    /* KEY_F1 */         0x05,
    /* KEY_F2 */         0x06,
    /* KEY_F3 */         0x04,
    /* KEY_F4 */         0x0c,
    /* KEY_F5 */         0x03,
    /* KEY_F6 */         0x0b,
    /* KEY_F7 */         0x83,
    /* KEY_F8 */         0x0a,
    /* KEY_F9 */         0x01,
    /* KEY_F10 */        0x09,
    /* KEY_NUMLOCK */    0x77,
    /* KEY_SCROLLLOCK */ 0x7e,
    /* KEY_KP7 */        0x6c,
    /* KEY_KP8 */        0x75,
    /* KEY_KP9 */        0x7d,
    /* KEY_KPMINUS */    0x7b,
    /* KEY_KP4 */        0x6b,
    /* KEY_KP5 */        0x73,
    /* KEY_KP6 */        0x74,
    /* KEY_KPPLUS */     0x79,
    /* KEY_KP1 */        0x69,
    /* KEY_KP2 */        0x72,
    /* KEY_KP3 */        0x7a,
    /* KEY_KP0 */        0x70,
    /* KEY_KPDOT */      0x71,
    /* KEY_ZENKAKUHANKAKU */ 0x00,
    /* KEY_102ND */      0x00,
    /* KEY_F11 */        0x78,
    /* KEY_F12 */        0x07,
    /* KEY_RO */         0x00,
    /* KEY_KATAKANA */   0x00,
    /* KEY_HIRAGANA */   0x00,
    /* KEY_HENKAN */     0x00,
    /* KEY_KATAKANAHIRAGANA */ 0x00,
    /* KEY_MUHENKAN */   0x00,
    /* KEY_KPJPCOMMA */  0x00,
    /* KEY_KPENTER */    0xe05a,
    /* KEY_RIGHTCTRL */  0xe014,
    /* KEY_KPSLASH */    0xe04a,
    /* KEY_SYSRQ */      0x00,
    /* KEY_RIGHTALT */   0xe011,
    /* KEY_LINEFEED */   0x00,
    /* KEY_HOME */       0xe06c,
    /* KEY_UP */         0xe075,
    /* KEY_PAGEUP */     0xe07d,
    /* KEY_LEFT */       0xe06b,
    /* KEY_RIGHT */      0xe074,
    /* KEY_END */        0xe069,
    /* KEY_DOWN */       0xe072,
    /* KEY_PAGEDOWN */   0xe07a,
    /* KEY_INSERT */     0xe070,
    /* KEY_DELETE */     0xe071,
    /* KEY_MACRO */      0x00,
    /* KEY_MUTE */       0xe023,
    /* KEY_VOLUMEDOWN */ 0xe021,
    /* KEY_VOLUMEUP */   0xe032,
    /* KEY_POWER */      0xe05e,
    /* KEY_KPEQUAL */    0x00,
    /* KEY_KPPLUSMINUS */ 0x00,
    /* KEY_PAUSE */      0x00,
    /* KEY_SCALE */      0x00,
    /* KEY_KPCOMMA */    0x00,
    /* KEY_HANGEUL */    0x00,
    /* KEY_HANGUEL */    0x00,
    /* KEY_HANJA */      0x00,
    /* KEY_YEN */        0x00,
    /* KEY_LEFTMETA */   0xe01f,
    /* KEY_RIGHTMETA */  0xe027,
    /* KEY_COMPOSE */    0x00
};

int _ps2keymap_get_press_bytes(int key)
{
    if (key == KEY_SYSRQ)
        return 4;
    else if (key == KEY_PAUSE)
        return 8;

    if (!ps2keymap_has_key(key))
        return 0;

    uint16_t press = ps2keymap_press[key];
    if ((press & 0xff00) >> 8 == 0xe0)
        return 2;
    return 1;
}

int _ps2keymap_get_break_bytes(int key)
{
    if (key == KEY_SYSRQ)
        return 6;

    if (!ps2keymap_has_key(key))
        return 0;

    uint16_t press = ps2keymap_press[key];
    if ((press & 0xff00) >> 8 == 0xe0)
        return 3;
    return 2;
}


int ps2keymap_has_key(int key)
{
    // Has special implementation
    if (key == KEY_SYSRQ || key == KEY_PAUSE)
        return 1;
    // Unimplemented
    if (key > KEY_COMPOSE)
        return 0;

    if (ps2keymap_press[key] == 0x00)
        return 0;
    return 1;
}

int ps2keymap_get_press(int key, int size, void* out)
{
    if (!ps2keymap_has_key(key))
        return -2;

    int bytes = _ps2keymap_get_press_bytes(key);
    if (bytes < 0)
        return -1;
    if (bytes > size)
        return -3;

    char* write = out;
    if (key == KEY_SYSRQ)
    {
        *(write++) = 0xe0;
        *(write++) = 0x12;
        *(write++) = 0xe0;
        *(write++) = 0x7c;
    }
    else if (key == KEY_PAUSE)
    {
        *(write++) = 0xe0;
        *(write++) = 0x14;
        *(write++) = 0x77;
        *(write++) = 0xe1;
        *(write++) = 0xf0;
        *(write++) = 0x14;
        *(write++) = 0xe0;
        *(write++) = 0x77;
    }
    else
    {
        int press = ps2keymap_press[key];
        for (int i = bytes; i > 0; --i)
        {
            *(write++) = (char)((press >> (8 * (i-1))) & 0xff);
        }
    }

    return bytes;
}

int ps2keymap_get_break(int key, int size, void* out)
{
    if (!ps2keymap_has_key(key))
        return -2;
    if (key == KEY_PAUSE)
        return -1;

    int bytes = _ps2keymap_get_break_bytes(key);
    if (bytes < 0)
        return -1;
    if (bytes > size)
        return -3;

    char* write = out;
    if (key == KEY_SYSRQ)
    {
        *(write++) = 0xe0;
        *(write++) = 0xf0;
        *(write++) = 0x7c;
        *(write++) = 0xe0;
        *(write++) = 0xf0;
        *(write++) = 0x12;
    }
    else
    {
        int press = ps2keymap_press[key];
        if ((press & 0xff00) >> 8 == 0xe0)
            *(write++) = (char)(0xe0);
        *(write++) = (char)(0xf0);
        *(write++) = (char)(press & 0xff);
    }

    return bytes;
}
