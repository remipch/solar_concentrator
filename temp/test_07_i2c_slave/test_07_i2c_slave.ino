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
const int I2C_BUFFER_SIZE = 20;



void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(19200);           // start serial for output
  
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
  int next_sep_pos = command.indexOf(sep,pos);
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
    int period_ms = parseCommandValue(command, 0, 10);      
    int samples_count = parseCommandValue(command, 1, 10);      
    setSampling(period_ms, samples_count);
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
    
    startOutputCommand(motor_pins, max_time_ms, threshold);
  }
  else if(command.startsWith("m:")) { // Measure buffer    
    int printed_samples_count = parseCommandValue(command, 0, 10);
    printMeasureBuffer(printed_samples_count);
  }
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
  if(command_size>=I2C_BUFFER_SIZE) {
    Serial.println("Too many char received");
    return;
  }

  char commands[I2C_BUFFER_SIZE];
  for(int i=0;i<command_size;i++) {
    commands[i] = Wire.read();
  }
  commands[command_size] = '\0';
  executeCommands(commands);
}

