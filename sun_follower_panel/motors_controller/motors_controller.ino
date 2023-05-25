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

const int I2C_DEVICE_ADDRESS = 4;


void printRam() {
	extern unsigned int __data_start;
	extern unsigned int __bss_start;
	extern unsigned int __heap_start;
	extern void *__brkval;	
	byte stack_start;	
	unsigned int heap_end = (__brkval == 0 ? (unsigned int) &__heap_start : (unsigned int) __brkval);
	unsigned int stackSize = RAMEND - (unsigned int)&stack_start + 1;	
	unsigned int freeMem = ((unsigned int)&stack_start) - heap_end;
	
	Serial.print(F("data : "));
	Serial.print((unsigned int)&__bss_start - (unsigned int)&__data_start);
	Serial.print(F(" byte(s) from 0x"));
	Serial.println((unsigned int)&__data_start,16);
	
	Serial.print(F("bss : "));
	Serial.print((unsigned int)&__heap_start - (unsigned int)&__bss_start);
	Serial.print(F(" byte(s) from 0x"));
	Serial.println((unsigned int)&__bss_start,16);
	
	Serial.print(F("heap : "));
	Serial.print(heap_end - (unsigned int)&__heap_start);
	Serial.print(F(" byte(s) from 0x"));
	Serial.println((unsigned int)&__heap_start,16);
	
	Serial.print(F("stack : "));
	Serial.print(stackSize);
	Serial.print(F(" byte(s) from 0x"));
	Serial.println((unsigned int)&stack_start,16);
		
	Serial.print(F("free RAM : "));
	Serial.print(freeMem);
	Serial.println(F(" bytes"));
	Serial.println();
}

void setup()
{
  Wire.begin(I2C_DEVICE_ADDRESS);
  //digitalWrite(A4, LOW);
  //digitalWrite(A5, LOW);
  
  Wire.onReceive(i2cReceivedEvent);
  //Wire.onRequest(i2cRequestEvent);
  Serial.begin(19200);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(UP_A_PIN, OUTPUT);
  pinMode(UP_B_PIN, OUTPUT);
  pinMode(DR_A_PIN, OUTPUT);
  pinMode(DR_B_PIN, OUTPUT);
  pinMode(DL_A_PIN, OUTPUT);
  pinMode(DL_B_PIN, OUTPUT);
  
  printRam();
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
  delay(10);
}

void i2cReceivedEvent(int command_size)
{
  if(command_size+1>=I2C_BUFFER_SIZE) {
    Serial.println("I2C buffer full");
    return;
  }

  char commands[I2C_BUFFER_SIZE];
  for(int i=0;i<command_size;i++) {
    commands[i] = Wire.read();
    Serial.println((int)commands[i]);
  }
  commands[command_size] = '\0';
  Serial.println(commands);
  parseCommands(commands);
}

void i2cRequestEvent()
{
  Wire.write(isRunning() ? 1 : 0);
}

