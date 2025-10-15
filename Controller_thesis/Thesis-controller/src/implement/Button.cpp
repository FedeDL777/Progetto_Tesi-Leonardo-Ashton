#include "include/Button.h"	
#include "Arduino.h"




Button::Button(int pin, bool usePullup, unsigned long debounceTime)
{
    
    
        this->pin = pin;
        this->debounceTime = debounceTime;
        this->usePullup = usePullup;

        if (usePullup)
            pinMode(pin, INPUT_PULLUP);
        else
            pinMode(pin, INPUT);

        lastStableState = readRaw();
        lastReading = lastStableState;
        lastDebounceTime = millis();
}
void Button::update()
{
    {
        bool currentReading = readRaw();

        if (currentReading != lastReading)
            lastDebounceTime = millis();

        if ((millis() - lastDebounceTime) > debounceTime)
        {
            if (currentReading != lastStableState)
            {
                lastStableState = currentReading;
                if (isPressed())
                    pressedEvent = true;
            }
        }

        lastReading = currentReading;
    }
}
bool Button::isPressed()
{
    return usePullup ? !lastStableState : lastStableState;
}

bool Button::wasPressed()
    {
        if (pressedEvent)
        {
            pressedEvent = false;
            return true;
        }
        return false;
    }

bool Button::readRaw() { 
        return digitalRead(this->pin); 
}