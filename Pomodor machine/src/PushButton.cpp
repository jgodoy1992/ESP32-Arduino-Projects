#include "PushButton.h"

PushButton::PushButton(byte bPin, bool isPullUp, bool internalPullUpActivated)
{
  this->bPin = bPin;
  this->isPullUp = isPullUp;
  this->internalPullUpActivated = internalPullUpActivated;

  lastTimeDebounce = millis();
  debounceDelay = 15;
}

void PushButton::init()
{
  if (isPullUp && internalPullUpActivated)
  {
    pinMode(bPin, INPUT_PULLUP);
  }
  else
  {
    pinMode(bPin, INPUT);
  }

  state = digitalRead(bPin);
}

void PushButton::readState()
{
  unsigned long currentTime = millis();

  if (currentTime - lastTimeDebounce > debounceDelay)
  {
    byte newButtonState = digitalRead(bPin);
    if (newButtonState != state)
    {
      state = newButtonState;
      lastTimeDebounce = currentTime;
    }
  }
}

bool PushButton::isPressed()
{
  readState();
  if (isPullUp)
  {
    return (state == LOW);
  }
  else
  {
    return (state == HIGH);
  }
}
