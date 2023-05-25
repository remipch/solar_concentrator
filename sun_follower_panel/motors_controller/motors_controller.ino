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

// I2C lib is limited in input buffer size
// Use the limit as default buffer size
const int I2C_BUFFER_SIZE = 20;

void setup()
{
  //Wire.begin(4);                    // join i2c bus with address #4
  //Wire.onReceive(i2cReceivedEvent); 
  Serial.begin(19200);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(UP_A_PIN, OUTPUT);
  pinMode(UP_B_PIN, OUTPUT);
  pinMode(DR_A_PIN, OUTPUT);
  pinMode(DR_B_PIN, OUTPUT);
  pinMode(DL_A_PIN, OUTPUT);
  pinMode(DL_B_PIN, OUTPUT);
  
  printSampling();
  
  stopCommands();
}

void loop()
{
  if(Serial.available()>0) {
    String commands = Serial.readStringUntil('\n');
    if(commands.length()>0) {
      Serial.print("parse commands: ");
      Serial.println(commands);
      parseCommands(commands);
    }    
  }
  
  // Start/update/terminate commands depending on command buffer and current command state
  run();
}

void i2cReceivedEvent(int command_size)
{
  Serial.println("i2cReceivedEvent");
  
  if(command_size>=I2C_BUFFER_SIZE) {
    Serial.println("I2C buffer full");
    return;
  }

  char commands[I2C_BUFFER_SIZE];
  for(int i=0;i<command_size;i++) {
    commands[i] = Wire.read();
  }
  commands[command_size] = '\0';
  parseCommands(commands);
}

