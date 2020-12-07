PS/2 keyboard/mouse emulator
====

A small application to bit-bash PS/2 over GPIO outputs, suitable for a Raspberry Pi or similar.


Components
----------

`ps2dev`

An emulated PS/2 device, marked by two pins; clock and data.

`ps2input`

A mapping between a Linux event device and an emulated PS/2 device.
