#include "Enums.h"

float time_changing_tab = 0, prev_ypr[3] = { 0, 0, 0 };




/* TODO: Check if positive or negative is correct for each axis.
OFF transitions:
  NEUTRAL (when device is turned up (for at least a certain time? maybe 300ms?))
  OFF (default)

NEUTRAL transitions 
  IN_TAB_MENU_FORWARD: Negative yaw (left)
  IN_TAB_MENU_BACKWARD: Positive yaw (right)
  CHANGE_WS_FORWARD: Negative yaw AND Negative pitch (up left)
  CHANGE_WS_BACKWARD: Positive yaw AND Positive pitch (up right)
  MIN_TABS: Negative yaw AND Negative pitch (down left)
  MAX_TABS: Nevative yaw AND Negative pitch (down left)
  WORKSPACE_MENU: Negative pitch (down)

  down right is unbound

  Alternative: MIN_TABS and MAX_TABS: Clockwise or Counter-Clockwise spin
*/

directions joystick_direction = directions::NONE;

SensorState currentState = SensorState::NEUTRAL;

bool are_tabs_minimized = false;

// True if in workspace menu or chaning workspaces when selecting a tab from workspace menu
bool is_doing_workspace_menu_operation = false;
bool is_going_to_specific_workspace = false;


int selected_workspace = 1;


int total_workspaces = 6;

int target_workspace = 1;
bool has_target_workspace = false;


float TAB_CHANGE_INTERVAL = 900;
float WORKSPACE_CHANGE_INTERVAL = 900;
float WORKSPACE_MENU_CHANGE_INTERVAL = 500;

float target_squared_yaw_pitch_angle = 50;

//bool workspace_menu_has_gone_up = false;

enum SensorState state_machine_joystick(SensorState senState, directions joystickDirection);

void reset_to_normal_state(){
  is_doing_workspace_menu_operation = false;
  is_going_to_specific_workspace = false;

  has_target_workspace = false;

  time_changing_tab = 0;

  release_all();
}

enum SensorState state_machine_joystick(SensorState senState, directions joystickDirection) {
  /*
  
  OFF,
  NEUTRAL,
  IN_TAB_MENU_FORWARD,
  IN_TAB_MENU_BACKWARD, 
  //ENTER_TAB,
  CHANGE_WS_FORWARD,
  CHANGE_WS_BACKWARD,
  MIN_TABS,
  MAX_TABS,
  WORKSPACE_MENU
  */

  switch (senState){
    case SensorState::OFF:
      switch (joystickDirection){
        case directions::NONE:
          return SensorState::NEUTRAL;
        default:
          return SensorState::OFF;
      }
    case SensorState::NEUTRAL:
      switch (joystickDirection){
          case directions::NONE:
            return SensorState::NEUTRAL;
      case directions::UP:
        return SensorState::OFF;
      case directions::UP_RIGHT:
        return SensorState::CHANGE_WS_FORWARD;
      case directions::RIGHT:
        return SensorState::IN_TAB_MENU_FORWARD;
      case directions::DOWN_RIGHT:
        return SensorState::NEUTRAL;
      case directions::DOWN:
        return SensorState::WORKSPACE_MENU;
      case directions::DOWN_LEFT:
        if (!are_tabs_minimized)
          return SensorState::MIN_TABS;
        else
          return SensorState::MAX_TABS;
      case directions::LEFT:
        return SensorState::IN_TAB_MENU_BACKWARD;
      case directions::UP_LEFT:
        return SensorState::CHANGE_WS_BACKWARD;
      }
  case SensorState::IN_TAB_MENU_FORWARD:
    switch (joystickDirection){
        case directions::NONE:
          return SensorState::NEUTRAL;
      default:
        return SensorState::IN_TAB_MENU_FORWARD;
    }
  case SensorState::IN_TAB_MENU_BACKWARD:
    switch (joystickDirection){
      case directions::NONE:
        return SensorState::NEUTRAL;
      default:
        return SensorState::IN_TAB_MENU_BACKWARD;
    }
  case SensorState::CHANGE_WS_FORWARD:
    switch (joystickDirection){
      case directions::NONE:
        return SensorState::NEUTRAL;
      default:
        return SensorState::CHANGE_WS_FORWARD;
    }
  case SensorState::CHANGE_WS_BACKWARD:
    switch (joystickDirection){
      case directions::NONE:
        return SensorState::NEUTRAL;
      default:
        return SensorState::CHANGE_WS_BACKWARD;
    }
  case SensorState::MIN_TABS:
    switch (joystickDirection){
      case directions::NONE:
        return SensorState::NEUTRAL;
      default:
        return SensorState::MIN_TABS;
    }
  case SensorState::MAX_TABS:
    switch (joystickDirection){
      case directions::NONE:
        return SensorState::NEUTRAL;
      default:
        return SensorState::MAX_TABS;
    }
  case SensorState::WORKSPACE_MENU:
    switch (joystickDirection){
      case directions::UP:
        return SensorState::OFF;
      case directions::DOWN_LEFT:
        has_target_workspace = true;
        target_workspace = 1;
        return SensorState::WORKSPACE_MENU;
      case directions::LEFT:
        has_target_workspace = true;
        target_workspace = 2;
        return SensorState::WORKSPACE_MENU;
      case directions::UP_LEFT:
        has_target_workspace = true;
        target_workspace = 3;
        return SensorState::WORKSPACE_MENU;
      case directions::UP_RIGHT:
        has_target_workspace = true;
        target_workspace = 4;
        return SensorState::WORKSPACE_MENU;
      case directions::RIGHT:
        has_target_workspace = true;
        target_workspace = 5;
        return SensorState::WORKSPACE_MENU;
      case directions::DOWN_RIGHT:
        has_target_workspace = true;
        target_workspace = 6;
        return SensorState::WORKSPACE_MENU;
      case directions::NONE:
        return SensorState::WORKSPACE_MENU;
      default:
        return currentState;//SensorState::ERROR;
    }
    default:
      return currentState;//SensorState::ERROR;
  }
  return currentState;//SensorState::ERROR;
}

void setup() {
  // Init serial output
  Serial.begin(OUTPUT__BAUD_RATE);

  // Init status LED
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  // Init sensors
  delay(50);  // Give sensors enough time to start
  I2C_Init();
  Accel_Init();
  Magn_Init();
  Gyro_Init();

  // Read sensors, init DCM algorithm
  delay(20);  // Give sensors enough time to collect data
  reset_sensor_fusion();
}

void loop() {

  // Time to read the sensors again?
  if ((millis() - timestamp) >= OUTPUT__DATA_INTERVAL) {
    timestamp_old = timestamp;
    timestamp = millis();
    float time_delta = timestamp - timestamp_old;
    if (timestamp > timestamp_old)
      G_Dt = (float)(timestamp - timestamp_old) / 1000.0f;  // Real time of loop run. We use this on the DCM algorithm (gyro integration time)
    else G_Dt = 0;

    // Update sensor readings
    read_sensors();

    // Apply sensor calibration
    compensate_sensor_errors();

    // Run DCM algorithm
    Compass_Heading();  // Calculate magnetic heading
    Matrix_update();
    Normalize();
    Drift_correction();
    Euler_angles();

    output_angles();

    /*
  
directions joystick_direction = NONE;

SensorState currentState = NEUTRAL; 

bool are_tabs_minimized = false;

int selected_tab = -1;

int max_tabs = 6;

  */

    directions prevJoyDir = joystick_direction;

    float ypr[3] = { TO_DEG(yaw), TO_DEG(pitch), TO_DEG(roll) };

    float pitchDeg = -ypr[1];
    float rollDeg = -ypr[2];

    float absoluteAngle = rollDeg / pitchDeg;
    if (absoluteAngle < 0)
      absoluteAngle = -absoluteAngle;

    float squared_yaw_pitch_angle = sqrt(pitchDeg * pitchDeg + rollDeg * rollDeg);

    
    if(squared_yaw_pitch_angle < target_squared_yaw_pitch_angle){
      joystick_direction = directions::NONE;
    }
    else{
      if(absoluteAngle > 2)//1.732)
      {
        if(rollDeg > 0){
          joystick_direction = directions::UP;
        }
        else{
          joystick_direction = directions::DOWN;
        }
      }
      else if (absoluteAngle < 0.53){//0.577){
        if(pitchDeg > 0){
          joystick_direction = directions::RIGHT;
        }
        else{
          joystick_direction = directions::LEFT;
        }
      }
      else{
        if(pitchDeg > 0 && rollDeg > 0){
          joystick_direction = directions::UP_RIGHT;
        }
        else if(pitchDeg < 0 && rollDeg > 0){
          joystick_direction = directions::UP_LEFT;
          
        }
        else if(pitchDeg > 0 && rollDeg < 0){
          joystick_direction = directions::DOWN_RIGHT;
        }
        else{
          joystick_direction = directions::DOWN_LEFT;
        }
      }
    }

    SensorState prevState = currentState;

    currentState = state_machine_joystick(currentState, joystick_direction);

    
    Serial.print("joystick direction: ");
    Serial.println(joystick_direction);
    
    Serial.print("Current state: ");
    Serial.println(currentState);

    switch (currentState){
      //case NEUTRAL:
      case SensorState::IN_TAB_MENU_FORWARD:
        if(prevState == SensorState::NEUTRAL){
          time_changing_tab = 0;
          start_tab_menu(true);
        }
        else if(prevState == SensorState::IN_TAB_MENU_FORWARD){
          time_changing_tab += time_delta;
          while(time_changing_tab >= TAB_CHANGE_INTERVAL){
            time_changing_tab -= TAB_CHANGE_INTERVAL;
            change_tab_in_menu(true);
          }
        }
        break;
      case SensorState::IN_TAB_MENU_BACKWARD:
        if(prevState == SensorState::NEUTRAL){
          time_changing_tab = 0;
          start_tab_menu(false);
        }
        else if(prevState == SensorState::IN_TAB_MENU_BACKWARD){
          time_changing_tab += time_delta;
          while(time_changing_tab >= TAB_CHANGE_INTERVAL){
            time_changing_tab -= TAB_CHANGE_INTERVAL;
            change_tab_in_menu(false);
          }
        }
        break;
      case SensorState::CHANGE_WS_FORWARD:
        if(prevState == SensorState::NEUTRAL){
          time_changing_tab = 0;
          change_workspace(true);
          if(selected_workspace < total_workspaces)
            selected_workspace += 1;
        }
        else if(prevState == SensorState::CHANGE_WS_FORWARD){
          time_changing_tab += time_delta;
          while(time_changing_tab >= TAB_CHANGE_INTERVAL){
            time_changing_tab -= TAB_CHANGE_INTERVAL;
            change_workspace(true);
            if(selected_workspace < total_workspaces)
              selected_workspace += 1;
          }
        }
        break;
      case SensorState::CHANGE_WS_BACKWARD:
        if(prevState == SensorState::NEUTRAL){
          time_changing_tab = 0;
          change_workspace(false);
          if(selected_workspace > 0)
            selected_workspace -= 1;
        }
        else if(prevState == SensorState::CHANGE_WS_BACKWARD){
          time_changing_tab += time_delta;
          while(time_changing_tab >= TAB_CHANGE_INTERVAL){
            time_changing_tab -= TAB_CHANGE_INTERVAL;
            change_workspace(false);
            if(selected_workspace > 0)
              selected_workspace -= 1;
          }
        }
        break;
      case SensorState::MIN_TABS:
        if(prevState == SensorState::NEUTRAL){
          are_tabs_minimized = true;
          minimize_all_tabs();
        }
        break;
      case SensorState::MAX_TABS:
        if(prevState == SensorState::NEUTRAL){
          are_tabs_minimized = false;
          maximize_all_tabs();
        }
        break;
      case SensorState::NEUTRAL:
        if(prevState != SensorState::WORKSPACE_MENU)
          reset_to_normal_state();
        break;
        
      case SensorState::WORKSPACE_MENU:
        if(prevState == SensorState::NEUTRAL){
          if(!is_doing_workspace_menu_operation){
            is_doing_workspace_menu_operation = true;
            is_going_to_specific_workspace = false;
            //toggle_workspace_menu();
          }
        }
        else if(prevState == SensorState::WORKSPACE_MENU){
          if(is_doing_workspace_menu_operation && !is_going_to_specific_workspace && has_target_workspace){
            has_target_workspace = false;
            int curWorkspace = move_workspace(total_workspaces, selected_workspace, target_workspace);
            selected_workspace = curWorkspace;
            if(selected_workspace != target_workspace){
              is_going_to_specific_workspace = true;
            }
            else{
              time_changing_tab = 0;
              is_going_to_specific_workspace = false;
              is_doing_workspace_menu_operation = false;
              currentState = SensorState::OFF;
            }
          }
          else if(is_going_to_specific_workspace){
            time_changing_tab += time_delta;
            while(time_changing_tab >= WORKSPACE_MENU_CHANGE_INTERVAL){
              time_changing_tab -= WORKSPACE_MENU_CHANGE_INTERVAL;
              int curWorkspace = move_workspace(total_workspaces, selected_workspace, target_workspace);
              selected_workspace = curWorkspace;
              if(selected_workspace == target_workspace){
                is_going_to_specific_workspace = false;
                is_doing_workspace_menu_operation = false;
                currentState = SensorState::OFF;
              }
            }
          }
        }
        break;
    }

    Serial.print("Current ws: ");
    Serial.println(selected_workspace);

      /*
    float ypr[3] = {TO_DEG(yaw), TO_DEG(pitch), TO_DEG(roll)};

    if (is_first_tab_change(false, ypr, prev_ypr[1])) {
      sensorState = IN_TAB_MENU_BACKWARD;
      tab_menu(false);
    }
    else if (is_first_tab_change(true, ypr, prev_ypr[1])) {
      sensorState = IN_TAB_MENU_FORWARD;
      tab_menu(true);
    }
    else if (sensorState == IN_TAB_MENU_BACKWARD || sensorState == IN_TAB_MENU_FORWARD) 
      if (!is_tab_change_in_menu(sensorState == IN_TAB_MENU_FORWARD, ypr, prev_ypr[1])) 
        sensorState = ENTER_TAB;
    
    if (sensorState != IN_TAB_MENU_BACKWARD && sensorState != IN_TAB_MENU_FORWARD && sensorState != ENTER_TAB) {
      if (is_changing_workspace(true, ypr, prev_ypr[2])) 
        sensorState = CHANGE_WS_FORWARD;
      else if (is_changing_workspace(false, ypr, prev_ypr[2]))
        sensorState = CHANGE_WS_BACKWARD;
      if (sensorState != CHANGE_WS_FORWARD && sensorState != CHANGE_WS_BACKWARD) {
        if (is_minimizing_tabs(ypr, prev_ypr[0]))
          sensorState = MIN_TABS;
        else if (is_maximizing_tabs(ypr, prev_ypr[0])) 
          sensorState = MAX_TABS;
      }
    }

    state_machine(time_delta);
    */


    prev_ypr[0] = ypr[0];
    prev_ypr[1] = ypr[1];
    prev_ypr[2] = ypr[2];
  }
}
