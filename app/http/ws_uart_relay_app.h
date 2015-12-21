#ifndef __WS_GAMEPAD_APP_H
#define __WS_GAMEPAD_APP_H

/*
Frame is composed of:
    address : array of bytes
    separator : byte : special character
    message type : byte : {command, request, response}    
    Message [
        size
        body
    ]
    
    

    message id : byte : for requests, 0 otherwise
    
    


*/

enum msg_header_type { RANDOM, IMMEDIATE, SEARCH };

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