#include "Keyboard.h"

// Enters tab menu by changing to either the next tab or the previous tab according to the "front" parameter
void tab_menu(bool front) {
  Keyboard.press(KEY_LEFT_ALT);
  change_tab_in_menu(front);
}
// Checks if yaw, pitch and roll of specific iteration corresponds to the conditions for entering tab menu
bool is_first_tab_change (bool front, float* ypr, float prevPitch) {
  return !front ? ypr[1] >= 50 && prevPitch < 50 : ypr[1] <= -50 && prevPitch > -50;
}
// Changes tab in menu
void change_tab_in_menu(bool front) {
  if(!front)
    Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.press(KEY_TAB);
  Keyboard.release(KEY_TAB);
  Keyboard.release(KEY_LEFT_SHIFT);
}
// Checks if yaw, pitch and roll of specific iteration corresponds to the conditions for changing tab in tab menu
bool is_tab_change_in_menu (bool front, float* ypr, float prevPitch) {
  return !front ? ypr[1] >= 50 && prevPitch >= 50 : ypr[1] <= -50 && prevPitch <= -50;
}
// Leaves tab menu
void leave_tab_menu() {
  Keyboard.release(KEY_LEFT_ALT);
}

// Changes to next or previous back workspace according to front parameter
void change_workspace(bool front) {
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press(front ? KEY_RIGHT_ARROW : KEY_LEFT_ARROW);
  Keyboard.releaseAll();
}