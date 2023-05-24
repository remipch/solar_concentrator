




// Current running command parameters
static int current_max_time_ms;
static int current_threshold;
static int start_time_ms;

void startOutputCommand(int motor_pins, int max_time_ms, int threshold) {
  current_max_time_ms = max_time_ms;
  current_threshold = threshold;
  executeOutputCommand(motor_pins, max_time_ms, threshold);
}

void updateOutputCommand() {
}

bool isOutputCommandRunning() {
  return false;
}


// motor_pins is a bit mask with bits in order:
// bit 0 is motor UP pin A
// ..
// bit 5 is motor DOWN_LEFT pin B
void writeMotorCommands(int motor_pins) {
  digitalWrite(UP_A_PIN, (motor_pins & 1) ? HIGH : LOW);
  digitalWrite(UP_B_PIN, (motor_pins & 2) ? HIGH : LOW);
  digitalWrite(DR_A_PIN, (motor_pins & 4) ? HIGH : LOW);
  digitalWrite(DR_B_PIN, (motor_pins & 8) ? HIGH : LOW);
  digitalWrite(DL_A_PIN, (motor_pins & 16) ? HIGH : LOW);
  digitalWrite(DL_B_PIN, (motor_pins & 32) ? HIGH : LOW);
}

void executeOutputCommand(int motor_pins, int max_time_ms, int threshold) {
  digitalWrite(LED_BUILTIN, HIGH); 
  writeMotorCommands(motor_pins);
  unsigned long start_time_ms = millis();
  Serial.print("  start_time_ms=");
  Serial.println(start_time_ms);
  resetMeasureBuffer();
  
  int average;
  while(true) {
    average = addMeasureAndComputeAverage(analogRead(MEASURE_PIN));
    if(average>threshold) {
      Serial.println("  STOP because average");
      break;
    }
      
    delay(getPeriodMs());
    if(millis() - start_time_ms > max_time_ms) {
      Serial.println("  STOP because max_time");
      break;
    }
  }
  digitalWrite(LED_BUILTIN, LOW); 
  writeMotorCommands(0);
  Serial.print("  average=");
  Serial.println(average); 
}
