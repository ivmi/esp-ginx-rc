#ifndef __WS_GAMEPAD_APP_H
#define __WS_GAMEPAD_APP_H

typedef struct {
    signed short x;
    signed short y;
    unsigned short btns;
} Gamepad;

extern Gamepad gamepad;

#endif