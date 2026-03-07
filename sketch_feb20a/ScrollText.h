#ifndef ScrollText_h
#define ScrollText_h

#include "Arduino.h"

class ScrollText {
  public:
    ScrollText(String _text);
    void tick();
    void goToStart();
    String getText();
    void setText(String _text);
  private:
    String text;
    int pos;
    int ticks;
    int speed;
    bool enableScrolling;
    String textMemo;
};

#endif