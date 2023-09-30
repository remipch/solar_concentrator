#include <SoftwareSerial.h>

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

// Soft serial pins (connection with ESP32 master)
int SOFT_RX_PIN = 9;
int SOFT_TX_PIN = 8;

SoftwareSerial MasterSerial(SOFT_RX_PIN, SOFT_TX_PIN);

void setup()
{
  MasterSerial.begin(19200);
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
  stopAndClearCommands();
}

void loop()
{
  String commands = "";
  if(Serial.available()>0) {
    commands = Serial.readStringUntil('\n');
  }
  else if(MasterSerial.available()>0) {
    commands = MasterSerial.readStringUntil('\n');
  }
  if(commands.length()>0) {
    Serial.print("parse commands: ");
    Serial.println(commands);
    parseAndRunCommands(commands);
  }    
  
  // Start/update/terminate commands depending on command buffer and current command state
  run();
}

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
