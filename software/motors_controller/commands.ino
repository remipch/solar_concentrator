// Copyright (C) 2024 Rémi Peuchot
// This code is distributed under GNU GPL v3 license

// ================= Output command =================

// Execute all buffered commands sequentially

// Current running output command
unsigned long start_time_ms;
int max_time_ms;
int threshold;
bool output_command_running = false;
bool running = false;

void stopAndClearCommands()
{
    clearCommandBuffer();
    output_command_running = false;
    running = false;
    writeMotors(0);
    digitalWrite(LED_BUILTIN, LOW);
}

void startNextCommand()
{
    if (isCommandAvailable()) {
        digitalWrite(LED_BUILTIN, HIGH);
        running = true;
        String command = getNextCommand();
        parseAndStartCommand(command);
    } else {
        stopAndClearCommands();
    }
}

// Start/update/terminate commands depending on command buffer and current command state
void run()
{
    if (running) {
        if (output_command_running) {
            updateOutputCommand();
        } else {
            startNextCommand();
        }
    }
}

bool isRunning()
{
    return running;
}

void sendRunningState()
{
    Serial.print("  running state: ");
    Serial.println(running ? '1' : '0');
    MasterSerial.print(running ? '1' : '0');
}

void startOutputCommand(int motor_pins, int cmd_max_time_ms, int cmd_threshold)
{
    max_time_ms = cmd_max_time_ms;
    threshold = cmd_threshold;
    resetMeasureBuffer();
    writeMotors(motor_pins);
    start_time_ms = millis();
    output_command_running = true;
}

void updateOutputCommand()
{
    int average = addMeasureAndComputeAverage(analogRead(MEASURE_PIN));
    if (average > threshold) {
        Serial.print("  STOP because average ; average=");
        Serial.println(average);
        output_command_running = false;
        return;
    }

    if (millis() - start_time_ms > max_time_ms) {
        Serial.print("  STOP because max_time ; average=");
        Serial.println(average);
        output_command_running = false;
        return;
    }

    delay(getPeriodMs());
}

// motor_pins is a bit mask with bits in order:
// bit 0 is motor UP pin A
// ..
// bit 5 is motor DOWN_LEFT pin B
void writeMotors(int motor_pins)
{
    for (int i = 0; i < MOTOR_PINS_COUNT; i++) {
        digitalWrite(MOTOR_PINS[i], (motor_pins & (1 << i)) ? HIGH : LOW);
    }
}
