float time_changing_tab = 0, prev_ypr[3] = {0, 0, 0};


typedef enum {
  NEUTRAL,
  IN_TAB_MENU_FORWARD,
  IN_TAB_MENU_BACKWARD, 
  ENTER_TAB,
  CHANGE_WS_FORWARD,
  CHANGE_WS_BACKWARD,
  MIN_TABS,
  MAX_TABS
} sensor_state;

sensor_state sensorState = NEUTRAL; 

void state_machine(float timeDelta) {
  switch(sensorState) {
    case IN_TAB_MENU_FORWARD:
    case IN_TAB_MENU_BACKWARD: 
      if(time_changing_tab >= 1000) {
        change_tab_in_menu(sensorState == IN_TAB_MENU_FORWARD);
        time_changing_tab -= 1000;
      }
      else time_changing_tab += timeDelta; 
      break;
    case ENTER_TAB: 
      leave_tab_menu();
      time_changing_tab = 0;
      sensorState = NEUTRAL;
      break;
    case CHANGE_WS_FORWARD:
    case CHANGE_WS_BACKWARD:
      change_workspace(sensorState == CHANGE_WS_FORWARD);
      sensorState = NEUTRAL;
      break;
    case MIN_TABS:
      minimize_all_tabs();
      sensorState = NEUTRAL;
      break;
    case MAX_TABS:
      maximize_all_tabs();
      sensorState = NEUTRAL;
      break;
    case NEUTRAL: 
      break;
  }
}

void setup() {
  // Init serial output
  Serial.begin(OUTPUT__BAUD_RATE);
  
  // Init status LED
  pinMode (STATUS_LED_PIN, OUTPUT);
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
  if((millis() - timestamp) >= OUTPUT__DATA_INTERVAL)
  {
    timestamp_old = timestamp;
    timestamp = millis();
    float time_delta = timestamp - timestamp_old;
    if (timestamp > timestamp_old)
      G_Dt = (float) (timestamp - timestamp_old) / 1000.0f; // Real time of loop run. We use this on the DCM algorithm (gyro integration time)
    else G_Dt = 0;

    // Update sensor readings
    read_sensors();

    // Apply sensor calibration
    compensate_sensor_errors();
    
    // Run DCM algorithm
    Compass_Heading(); // Calculate magnetic heading
    Matrix_update();
    Normalize();
    Drift_correction();
    Euler_angles();
    
    output_angles();

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
    prev_ypr[0] = ypr[0];
    prev_ypr[1] = ypr[1];
    prev_ypr[2] = ypr[2];
  }
}
