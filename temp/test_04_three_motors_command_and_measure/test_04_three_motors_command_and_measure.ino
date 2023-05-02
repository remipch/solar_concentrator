
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

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(UP_A_PIN, OUTPUT);
  pinMode(UP_B_PIN, OUTPUT);
  pinMode(DR_A_PIN, OUTPUT);
  pinMode(DR_B_PIN, OUTPUT);
  pinMode(DL_A_PIN, OUTPUT);
  pinMode(DL_B_PIN, OUTPUT);
  
  Serial.begin(19200);
  Serial.println("Wait motors commands :");
  Serial.println("<motor_number> # 10 steps (1 s)");
  Serial.println("<motor_number>/<step_count> # number of steps (100 ms)");
  Serial.println("<motor_number>:<force_threshold> # until threshold reached");
}

void writeMotorCommands(bool up_a,bool up_b,bool dr_a,bool dr_b,bool dl_a,bool dl_b) {
  Serial.print("  command: ");
  Serial.print(up_a);
  Serial.print(up_b);
  Serial.print(dr_a);
  Serial.print(dr_b);
  Serial.print(dl_a);
  Serial.println(dl_b);
  digitalWrite(UP_A_PIN, up_a ? HIGH : LOW);
  digitalWrite(UP_B_PIN, up_b ? HIGH : LOW);
  digitalWrite(DR_A_PIN, dr_a ? HIGH : LOW);
  digitalWrite(DR_B_PIN, dr_b ? HIGH : LOW);
  digitalWrite(DL_A_PIN, dl_a ? HIGH : LOW);
  digitalWrite(DL_B_PIN, dl_b ? HIGH : LOW);
}

int threshold;
int remaing_steps;

// the loop function runs over and over again forever
void loop() {
  if(Serial.available() > 0) {		
    digitalWrite(LED_BUILTIN, HIGH);
    String input = Serial.readStringUntil('\n');
    Serial.print("input: ");
    Serial.println(input);
    
    int colIndex = input.lastIndexOf(':');
    int motor;
    if(colIndex>0) {
      String motor_str = input.substring(0,colIndex);
      motor_str.trim();
      motor = motor_str.toInt();
      String threshold_str = input.substring(colIndex+1);
      threshold_str.trim();
      threshold = threshold_str.toInt();
      remaing_steps = 100;
    }
    else {
      int slashIndex = input.lastIndexOf('/');
      if(slashIndex>0) {
        String motor_str = input.substring(0,slashIndex);
        motor_str.trim();
        motor = motor_str.toInt();
        String steps_str = input.substring(slashIndex+1);
        steps_str.trim();
        remaing_steps = steps_str.toInt();
        threshold = 1000;
      }
      else {
        input.trim();
        motor = input.toInt();
        threshold = 1000;
        remaing_steps = 10;
      }
    }
      
    Serial.print("  motor: ");
    Serial.println(motor);
    Serial.print("  threshold: ");
    Serial.println(threshold);
      
    if(motor==0)
      writeMotorCommands(0,0,0,0,0,0);
    else if(motor==2)
      writeMotorCommands(1,0,0,0,0,0);
    else if(motor==8)
      writeMotorCommands(0,1,0,0,0,0);
    else if(motor==7)
      writeMotorCommands(0,0,1,0,0,0);
    else if(motor==3)
      writeMotorCommands(0,0,0,1,0,0);
    else if(motor==9)
      writeMotorCommands(0,0,0,0,1,0);
    else if(motor==1)
      writeMotorCommands(0,0,0,0,0,1);
    else
      Serial.println("unknown motor");        
  }
  
  delay(100);
  
  if(remaing_steps>0) {
    Serial.print("    remaing_steps: ");
    Serial.println(remaing_steps);
    remaing_steps--;
  
    if(remaing_steps==0) {
      Serial.println("  STEPS REACHED");
      writeMotorCommands(0,0,0,0,0,0);
    }
  
    int measure = analogRead(MEASURE_PIN); 
    Serial.print("    measure: ");
    Serial.println(measure);
  
    if(measure > threshold) {
      Serial.println("  THRESHOLD REACHED");
      writeMotorCommands(0,0,0,0,0,0);
      remaing_steps = 0;
    }
  }
  
  digitalWrite(LED_BUILTIN, LOW);  
}

