
#ifndef __BUTTON__
#define __BUTTON__

#include "Arduino.h"

class Button
{
public:
    Button(int pin, bool usePullup = false, unsigned long debounceTime = 50);
    void update();
    bool isPressed();

    bool wasPressed();
    bool readRaw();
private:
    int pin;
    bool usePullup;
    bool lastReading;
    bool lastStableState;
    bool pressedEvent = false;
    unsigned long lastDebounceTime;
    unsigned long debounceTime;

};

#endif