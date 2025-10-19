#include "Arduino.h"

#ifndef __SETUP__
#define __SETUP__



#define YELLOW_BASE_SX 14 // GPIO14 → pulsante giallo sinistra del servo_base
#define YELLOW_BASE_DX 27  // GPIO27 → pulsante giallo destra del servo_base
#define WHITE_ELBOW_SX 26 // GPIO26 → pulsante bianco sinistra del gomito
#define WHITE_ELBOW_DX 25 // GPIO25 → pulsante bianco destra del gomito
#define BLUE_WRIST_SX 33  // GPIO33 → pulsante blu sinistra del polso
#define BLUE_WRIST_DX 32  // GPIO32 → pulsante blu destra del polso
uint8_t receiverAddress[] = {0x68, 0x25, 0xDD, 0x20, 0x00, 0xF0};




#endif
