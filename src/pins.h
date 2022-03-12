#ifndef PINS_H
#define PINS_H

#ifdef GAMEPAD_PCB_V2

#define J1 22
#define J2 23
#define J3 18
#define J4 25
#define J5 5
#define J6 16
#define J7 17
#define J8 4
#define J9 35
#define J10 19
#define J11 32
#define J12 33
#define J13 23
#define J14 26
#define J15 14
#define J16 12

#else

#define J1 4
#define J2 22
#define J3 5 // outputs PWM signal at boot
#define J4 23
#define J5 17
#define J6 16
#define J7 19
#define J8 13
#define J9 18
#define J10 14 // outputs PWM signal at boot
#define J11 21
#define J12 32
#define J13 15 // outputs PWM signal at boot
#define J14 12 // boot fail if pulled high (using for LED, so that's fine)
#define J15 33
#define J16 27

#endif

#endif