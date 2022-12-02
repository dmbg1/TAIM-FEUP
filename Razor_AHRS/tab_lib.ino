#include "Keyboard.h"

// Enters tab menu by changing to either the next tab or the previous tab according to the "front" parameter
void start_tab_menu(bool front) {
  Keyboard.press(KEY_LEFT_ALT);
  change_tab_in_menu(front);
}

/*
// Checks if yaw, pitch and roll of specific iteration corresponds to the conditions for entering tab menu
bool is_first_tab_change (bool front, float* ypr, float prevPitch) {
  return !front ? ypr[1] >= 50 && prevPitch < 50 : ypr[1] <= -50 && prevPitch > -50;
}
*/


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

// Checks if yaw, pitch and roll of specific iteration corresponds to the conditions for changing a workspace
bool is_changing_workspace (bool front, float* ypr, float prevRoll) {
  return !front ? ypr[2] >= 50 && prevRoll < 50 : ypr[2] <= -50 && prevRoll > -50;
}

void minimize_all_tabs() {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('m');
  Keyboard.releaseAll();
}

void maximize_all_tabs() {
  Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('m');
  Keyboard.releaseAll();
}

// Checks if yaw, pitch and roll of specific iteration corresponds to the conditions for minimizing all tabs
bool is_minimizing_tabs (float* ypr, float prevYaw) {
  return ypr[0] <= -130 && prevYaw > -130;
}

// Checks if yaw, pitch and roll of specific iteration corresponds to the conditions for maximizing all tabs
bool is_maximizing_tabs (float* ypr, float prevYaw) {
  return ypr[0] >= -10 && prevYaw < -10;
}

void toggle_workspace_menu(){
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press(KEY_TAB);
  Keyboard.releaseAll();
}

// Returns the tab it just moved to. Moves one workspace closer to the target. Chooses forward or backward depending on which one is possible
int move_workspace(int totalWorkspaces, int curWorkspace, int targetWorkspace){
  if(curWorkspace == targetWorkspace)
    return curWorkspace;

  int distance_forward = targetWorkspace - curWorkspace;//(curWorkspace > targetWorkspace) ? totalWorkspaces + targetWorkspace - curWorkspace : targetWorkspace - curWorkspace;

  int distance_backward = curWorkspace - targetWorkspace;//(curWorkspace > targetWorkspace) ? curWorkspace - targetWorkspace : totalWorkspaces + curWorkspace - targetWorkspace;

  if(distance_forward <= 0){
    distance_forward = 999999;
  }
  if(distance_backward <= 0){
    distance_backward = 999999;
  }

  if(distance_forward <= distance_backward){
    change_workspace(true);
    curWorkspace += 1;
    if(curWorkspace > totalWorkspaces){
      curWorkspace = 1;
    }
  }
  else{
    change_workspace(false);
    curWorkspace -= 1;
    if(curWorkspace <= 0){
      curWorkspace = totalWorkspaces;
    }
  }


  return curWorkspace;
}

void release_all(){
  Keyboard.releaseAll();
}

