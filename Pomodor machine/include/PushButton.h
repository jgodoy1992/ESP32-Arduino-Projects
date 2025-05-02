#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <Arduino.h>

class PushButton
{
private:
  byte bPin;
  byte state;

  bool isPullUp;
  bool internalPullUpActivated;

  unsigned long lastTimeDebounce;
  unsigned long debounceDelay;
  int buttonState;

  void readState();

public:
  PushButton() {}

  PushButton(byte bPin, bool isPullUp, bool internalPullUpActivated);
  void init();
  bool isPressed();
};

#endif
