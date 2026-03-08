#include "ScrollText.h"
#include "Arduino.h"

String getScrolledText(String text, int pos) {
  String out = text.substring(pos) + text.substring(0, pos);
  return out;
}

ScrollText::ScrollText(String _text) {
  text = _text;
  speed = 1;
  pos = 0;
  ticks = 0;
  enableScrolling = true;
  textMemo = _text;
}

void ScrollText::setText(String _text) {
  text = _text;
  ScrollText::goToStart();
}

void ScrollText::tick() {
  if (enableScrolling) {
    ticks++;
    if (ticks > speed) {
      ticks = 0;
      pos = (pos + 1) % text.length();
      textMemo = getScrolledText(text, pos);
    }
  }
}

void ScrollText::incSpeed() {
  speed = max(0, speed - 1);
}

void ScrollText::decSpeed() {
  speed = min(4, speed + 1);
}

void ScrollText::goToStart() {
  textMemo = text;
  pos = 0;
  ticks = 0;
}

String ScrollText::getText() {
  return textMemo;
}