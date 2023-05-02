
int PIN_A = 2;
int PIN_B = 3;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  
  Serial.begin(19200);
  Serial.println("Start blink");
}

// the loop function runs over and over again forever
void loop() {
  if(Serial.available() > 0) {		
    String input = Serial.readStringUntil('\n');
    Serial.print("read: ");
    Serial.println(input);
    if(input.length()==2) {
      char a = input.charAt(0);
      char b = input.charAt(1);
      Serial.print("  a = ");
      Serial.print(a);
      Serial.print("  b = ");
      Serial.println(b);
      
      if(a=='0')
        digitalWrite(PIN_A, LOW);
      else if(a=='1')
        digitalWrite(PIN_A, HIGH);
      
      if(b=='0')
        digitalWrite(PIN_B, LOW);
      else if(b=='1')
        digitalWrite(PIN_B, HIGH);
        
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

