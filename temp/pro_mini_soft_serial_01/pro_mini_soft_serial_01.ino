#include <SoftwareSerial.h>

SoftwareSerial mySerial(9, 8); // RX, TX

void setup()  
{
  Serial.begin(19200);

  // set the data rate for the SoftwareSerial port
  mySerial.begin(19200);
  Serial.println("Ready");
}

void loop() // run over and over
{
  if(mySerial.available()>0) {
    String message = mySerial.readStringUntil('\n');
    Serial.print("  message received from esp32: ");
    Serial.println(message);
    
    if(message.equals("s")) {
      char* reply = "1";
      mySerial.write(reply);
      Serial.println("reply sent");
    }
  }
}

