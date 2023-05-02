// Wire Slave Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI slave device
// Refer to the "Wire Master Writer" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(19200);           // start serial for output
  
  Serial.println("setup");
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  delay(100);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  if(howMany==2) {
    int address = Wire.read();
    Serial.print("address: ");
    Serial.println(address);
    
    char value = Wire.read();
    Serial.print("value: "); //    WTF !!! il faut commenter cette ligne
    Serial.println(value);
    //Serial.println();
    if(value=='0') {
      Serial.println("LED OFF");
      digitalWrite(LED_BUILTIN, LOW);  
    }
    else if(value=='1') {
      Serial.println("LED ON");
      digitalWrite(LED_BUILTIN, HIGH);  
    }
  }
  /*
  while(1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
  */
}
