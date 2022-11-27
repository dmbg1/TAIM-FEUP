#include "Keyboard.h"

bool IN_TAB_MENU = false;
void change_tab(bool front) {
    Keyboard.press(KEY_LEFT_ALT);
    if(!front)
      Keyboard.press(KEY_LEFT_SHIFT);
    Keyboard.press(KEY_TAB);
    Keyboard.releaseAll();
}

void tab_menu(bool front) {
  IN_TAB_MENU = true;
  Keyboard.press(KEY_LEFT_ALT);
  change_tab_in_menu(front);
}

void change_tab_in_menu(bool front) {
  if(!front)
    Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.press(KEY_TAB);
  Keyboard.release(KEY_TAB);
  Keyboard.release(KEY_LEFT_SHIFT);
}

void leave_tab_menu() {
  Keyboard.release(KEY_LEFT_ALT);
  IN_TAB_MENU = false;
}

void change_workspace(bool front) {
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press(front ? KEY_RIGHT_ARROW : KEY_LEFT_ARROW);
  Keyboard.releaseAll();
}