#ifndef __WS_GAMEPAD_APP_H
#define __WS_GAMEPAD_APP_H

typedef struct {
    signed short x;
    signed short y;
    unsigned short btns;
} Gamepad;

# pragma pack (1)
typedef struct {
    uint32_t header;
    uint16_t buttons;
    uint16_t switches;
    int16_t accel[3];
    int16_t tPad[4];
    uint8_t reserved[14];
} GPDataCmds;

extern Gamepad gamepad;
extern GPDataCmds gpDataCmds;

#endif