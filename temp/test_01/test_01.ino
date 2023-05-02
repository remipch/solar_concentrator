// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(19200);
  Serial.println("Start blink");
}

bool b=true;

// the loop function runs over and over again forever
void loop() {
  if(Serial.available() > 0) {		
    String input = Serial.readStringUntil('\n');
    Serial.print("read: ");
    Serial.println(input);
  }
  
  if(b) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  b = !b;
  delay(1000);
}

