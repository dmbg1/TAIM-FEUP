float prevYaw = 0, prevPitch = 0, prevRoll = 0,
time_not_changing_tab = 0, time_changing_forward = 0, time_changing_backwards = 0;

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
    if (ypr[1] <= 50 && prevPitch > 50) 
      IN_TAB_MENU ? change_tab_in_menu(false) :  tab_menu(false);
    else if (ypr[1] >= 50 && prevPitch >= 50) {
      if(time_changing_forward >= 1500) {
        change_tab_in_menu(false);
        time_changing_forward -= 1500;
      }
      else time_changing_forward += time_delta;
    }
    else if (ypr[1] >= -50 && prevPitch < -50) 
    IN_TAB_MENU ? change_tab_in_menu(true) :  tab_menu(true);
    else if (ypr[1] <= -50 && prevPitch <= -50) {
      if(time_changing_backwards >= 1500) {
        change_tab_in_menu(true);
        time_changing_backwards -= 1500;
      }
      else time_changing_backwards += time_delta;
    }
    else {
      if (time_changing_forward > 0) time_changing_forward = 0;
      else if (time_changing_backwards > 0) time_changing_backwards = 0;
      leave_tab_menu();
    }
    prevPitch = ypr[1];
  }
}
