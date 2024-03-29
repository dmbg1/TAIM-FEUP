

enum directions {
  NONE,
  UP,
  UP_RIGHT,
  RIGHT,
  DOWN_RIGHT,
  DOWN,
  DOWN_LEFT,
  LEFT,
  UP_LEFT
};

enum SensorState {
  ERROR,
  OFF,
  NEUTRAL,
  IN_TAB_MENU_FORWARD,
  IN_TAB_MENU_BACKWARD,
  CHANGE_WS_FORWARD,
  CHANGE_WS_BACKWARD,
  MIN_TABS,
  MAX_TABS,
  WORKSPACE_MENU
};

enum WorkspaceMenuState {
  OTHER_STATE,
  START,
  HAS_GONE_NEUTRAL,
  SELECTED_DIRECTION,
  CHANGING_WORKSPACES
};
