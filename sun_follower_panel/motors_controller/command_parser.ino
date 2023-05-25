// ================= Command parser =================

// Parse input command strings, append commands to buffer and start

// Parse the arguments part of a command string, return the nth argument as int
int parseCommandArgument(String command, int value_index, int default_value) {
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
void parseAndRunCommands(String commands){
  if(commands.length()==0) {
    return;
  }

  // First check for immediate single command  
  if(commands.startsWith("c")) { // Stop and clear all commands immediately
    stopAndClearCommands();
  }
  else if(commands.equals("s")) { // Return state immediately
    sendRunningState();
  }
  else if(commands.startsWith("p")) { // Print all commands immediately
    printCommandBuffer();
  }
  else if(commands.startsWith("m:")) { // Print measure buffer
    int printed_samples_count = parseCommandArgument(commands, 0, 10);
    printMeasureBuffer(printed_samples_count);
  }
  else { 
    // Parse commands, append them and start the first one    
    stopAndClearCommands();
    
    char sep = ';';
    int pos = 0;
    do {
      int next_sep_pos = commands.indexOf(sep, pos);
      String command = (next_sep_pos==-1) ? commands.substring(pos) : commands.substring(pos,next_sep_pos);
      if(command.length()>0) {
        appendCommand(command);
      }
      pos = next_sep_pos+1;
    } while(pos>0);
    
    // Start the first command
    if(isCommandAvailable()) {
      startNextCommand();
    }
  }
}

// Start command immediately
void parseAndStartCommand(String command) {  
  Serial.print("start command: ");
  Serial.println(command);
  
  if(command.startsWith("ms:")) { // Measure sampling    
    int period_ms = parseCommandArgument(command, 0, 10);
    int samples_count = parseCommandArgument(command, 1, 10);
    setSampling(period_ms, samples_count);
  }
  else if(command.startsWith("o:")) { // Output    
    int motor_pins = parseCommandArgument(command, 0, -1);
    Serial.print("  motor_pins: ");
    Serial.println(motor_pins);
    if(motor_pins==-1)
      return;
      
    int max_time_ms = parseCommandArgument(command, 1, 200);
    Serial.print("  max_time_ms: ");
    Serial.println(max_time_ms);
    
    int threshold = parseCommandArgument(command, 2, 1024);
    Serial.print("  threshold: ");
    Serial.println(threshold);
    
    startOutputCommand(motor_pins, max_time_ms, threshold);
  }
  else {
    Serial.println("unknown command: STOP");  
    stopAndClearCommands();
  }
}


