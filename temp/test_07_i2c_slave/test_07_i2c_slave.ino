#include <Wire.h>

// motor UP
int UP_A_PIN = 2;    // unroll
int UP_B_PIN = 3;    // roll

// motor DOWN_RIGHT
int DR_A_PIN = 4;    // unroll
int DR_B_PIN = 5;    // roll

// motor DOWN_LEFT
int DL_A_PIN = 6;    // unroll
int DL_B_PIN = 7;    // roll

int MEASURE_PIN = A0;

// Wire lib is limited in input buffer size
// Use the limit as default buffer size
const int COMMAND_BUFFER_SIZE = 20;

// Measure buffer
const int MAX_SAMPLES_COUNT = 500; // (1000 bytes on pro mini is almost half the RAM)
int measure_buffer[MAX_SAMPLES_COUNT];
unsigned long measure_sum;
int measure_index;

// Sampling config, set by "ms" command at runtime
int period_ms = 10;
int samples_count = 30;

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(19200);           // start serial for output
  
  Serial.println("setup");
  Serial.print("  period_ms: ");
  Serial.println(period_ms);
  Serial.print("  samples_count: ");
  Serial.println(samples_count);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(UP_A_PIN, OUTPUT);
  pinMode(UP_B_PIN, OUTPUT);
  pinMode(DR_A_PIN, OUTPUT);
  pinMode(DR_B_PIN, OUTPUT);
  pinMode(DL_A_PIN, OUTPUT);
  pinMode(DL_B_PIN, OUTPUT);
}

int parseCommandValue(String command, int value_index, int default_value) {
  char sep = ':';
  int pos = 0;
  for(int i=0 ; i<value_index+1 ; i++) {
    pos = command.indexOf(sep, pos);
    if(pos==-1)
      return default_value;
    sep = ',';
    pos++;
  }
  int next_sep_pos = command.indexOf(command,pos);
  String value = (next_sep_pos==-1) ? command.substring(pos) : command.substring(pos,next_sep_pos);
  return value.toInt();   
}

// Parse input string and execute all commands separated by ';' char
void executeCommands(String commands) {
  char sep = ';';
  int pos = 0;
  do {
    int next_sep_pos = commands.indexOf(sep, pos);
    String command = (next_sep_pos==-1) ? commands.substring(pos) : commands.substring(pos,next_sep_pos);
    executeCommand(command);
    pos = next_sep_pos+1;
  } while(pos>0);
}

void executeCommand(String command) {  
  if(command.length()==0) {
    return;
  }
  
  Serial.print("command: ");
  Serial.println(command);
  
  if(command.startsWith("ms:")) { // Measure sampling    
    period_ms = parseCommandValue(command, 0, 10);
    Serial.print("  period_ms: ");
    Serial.println(period_ms);
      
    samples_count = parseCommandValue(command, 1, 10);
    if(samples_count > MAX_SAMPLES_COUNT)
      samples_count = MAX_SAMPLES_COUNT;
    Serial.print("  samples_count: ");
    Serial.println(samples_count);
  }
  else if(command.startsWith("o:")) { // Output    
    int motor_pins = parseCommandValue(command, 0, -1);
    Serial.print("  motor_pins: ");
    Serial.println(motor_pins);
    if(motor_pins==-1)
      return;
      
    int max_time_ms = parseCommandValue(command, 1, 200);
    Serial.print("  max_time_ms: ");
    Serial.println(max_time_ms);
    
    int threshold = parseCommandValue(command, 2, 1024);
    Serial.print("  threshold: ");
    Serial.println(threshold);
    
    executeOutputCommand(motor_pins, max_time_ms, threshold);
  }
  else if(command.startsWith("m:")) { // Measure buffer    
    int printed_samples_count = parseCommandValue(command, 0, 10);
    if(printed_samples_count > MAX_SAMPLES_COUNT)
      printed_samples_count = MAX_SAMPLES_COUNT;
    printMeasureBuffer(printed_samples_count);
  }
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
      
    delay(period_ms);
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

void resetMeasureBuffer() {
  measure_index = 0;
  measure_sum = 0;
  for(int i=0;i<MAX_SAMPLES_COUNT;i++) {
    measure_buffer[i] = 0;
  }
}  

void printMeasureBuffer(int printed_samples_count) {
  Serial.println("measure_buffer=");
  int start_index = measure_index + MAX_SAMPLES_COUNT - printed_samples_count; // always positive to avoid negative int modulo
  for(int i=0;i<printed_samples_count;i++) {
    Serial.println(measure_buffer[(start_index+i)%MAX_SAMPLES_COUNT]);
  }
}  

int addMeasureAndComputeAverage(int measure) {  
  int oldest_measure_index = (measure_index + MAX_SAMPLES_COUNT - samples_count) % MAX_SAMPLES_COUNT; // always positive to avoid negative int modulo
  measure_sum -= measure_buffer[oldest_measure_index];
  measure_sum += measure;
  measure_buffer[measure_index] = measure;
  
  // Cycling over the full buffer allow to cache all possible measures
  // It's only used for log purpose (see 'm' command)
  measure_index = (measure_index+1) % MAX_SAMPLES_COUNT;
  
  // Always divide by samples_count, so average value is under-evaluated when measure starts,
  // it's a simple/hacky way to guarantee that at least samples_count measures are taken before comparing with threshold
  return measure_sum / samples_count;
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

void loop()
{
  if(Serial.available()>0) {
    String commands = Serial.readStringUntil('\n');
    executeCommands(commands);
  }
  delay(10);
}

void receiveEvent(int command_size)
{
  if(command_size>=COMMAND_BUFFER_SIZE) {
    Serial.println("Too many char received");
    return;
  }

  char commands[COMMAND_BUFFER_SIZE];
  for(int i=0;i<command_size;i++) {
    commands[i] = Wire.read();
  }
  commands[command_size] = '\0';
  executeCommands(commands);
}

