
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

int RELAXING_STEPS = 10;

int TENSING_THRESHOLD = 15;

int STEP_DURATION_MS = 50;

int THRESHOLD_STEPS = 4; // nb of successive steps that must be over threshold

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
  Serial.println("<direction_number>");
  Serial.println("<direction_number>:<force_threshold>");
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
int motors_direction = -1;

void loop() {
  if(Serial.available() > 0) {	
    String input = Serial.readStringUntil('\n');
    Serial.print("input: ");
    Serial.println(input);
    
    int colIndex = input.lastIndexOf(':');
    int motor;
    if(colIndex>0) {
      String motor_str = input.substring(0,colIndex);
      motor_str.trim();
      motors_direction = motor_str.toInt();
      String threshold_str = input.substring(colIndex+1);
      threshold_str.trim();
      threshold = threshold_str.toInt();
    }
    else {
      input.trim();
      motors_direction = input.toInt();
      threshold = TENSING_THRESHOLD;
    }
    remaing_steps = RELAXING_STEPS;
      
    Serial.print("  motors_direction: ");
    Serial.println(motors_direction);
    Serial.print("  threshold: ");
    Serial.println(threshold);
    
    Serial.println("  START RELAXING PHASE");
    
    if(motors_direction==1)
      writeMotorCommands(1,0,0,0,0,0);
    else if(motors_direction==4)
      writeMotorCommands(0,0,1,0,0,0);
    else if(motors_direction==7)
      writeMotorCommands(0,0,1,0,0,0);
    else if(motors_direction==3)
      writeMotorCommands(1,0,0,0,0,0);
    else if(motors_direction==6)
      writeMotorCommands(0,0,0,0,1,0);
    else if(motors_direction==9)
      writeMotorCommands(0,0,0,0,1,0);
    else {
      Serial.println("unknown motors_direction");  
      writeMotorCommands(0,0,0,0,0,0);     
      motors_direction = -1;
    }
  }
  
  delay(STEP_DURATION_MS);
  digitalWrite(LED_BUILTIN, LOW);  
  
  if(motors_direction>0) { // RELAXING PHASE
    //int measure = analogRead(MEASURE_PIN); 
    //Serial.print("    measure: ");
    //Serial.println(measure);
    
    Serial.print("    remaing_steps: ");
    Serial.println(remaing_steps);
    remaing_steps--;
  
    if(remaing_steps==0) {
      
      Serial.println("  END RELAXING PHASE : START TENSING");	
      if(motors_direction==1)
        writeMotorCommands(0,0,0,0,0,1);
      else if(motors_direction==4)
        writeMotorCommands(0,0,0,0,0,1);
      else if(motors_direction==7)
        writeMotorCommands(0,1,0,0,0,0);
      else if(motors_direction==3)
        writeMotorCommands(0,0,0,1,0,0);
      else if(motors_direction==6)
        writeMotorCommands(0,0,0,1,0,0);
      else if(motors_direction==9)
        writeMotorCommands(0,1,0,0,0,0);
        
      motors_direction = 0; // tensing phase
      return;
    }
  }
  else if(motors_direction==0) { // TENSING PHASE    
    int measure = analogRead(MEASURE_PIN); 
    Serial.print("    measure: ");
    Serial.println(measure);
  
    if(measure > threshold) {
      remaing_steps++;
      if(remaing_steps >= THRESHOLD_STEPS) {
        Serial.println("  THRESHOLD REACHED : END TENSING PHASE");
        digitalWrite(LED_BUILTIN, HIGH);
        writeMotorCommands(0,0,0,0,0,0);
        motors_direction = -1;
      }
    }
    else {
      remaing_steps = 0;
    }
  }
  
}

