#include "button.h"

#ifndef MODE_H
#define MODE_H

class Mode {
  public:
    Mode() { }
    virtual ~Mode() { }

    virtual void setup() = 0;
    virtual void teardown() = 0; // this is here to support a maybe-future feature where you can hotswap modes without restarting
    virtual void pressButton(ButtonType button) = 0;
    virtual void releaseButton(ButtonType button) = 0;
};

#endif