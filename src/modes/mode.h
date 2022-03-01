#include "button.h"

#ifndef MODE_H
#define MODE_H

class Mode {
  public:
    Mode() { }
    virtual ~Mode() { }

    virtual void setup() = 0;
    virtual void teardown() = 0;
    virtual bool loop() = 0;
    virtual void pressButton(ButtonType button) = 0;
    virtual void releaseButton(ButtonType button) = 0;
};

#endif