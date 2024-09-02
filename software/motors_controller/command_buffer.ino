// Copyright (C) 2024 Rémi Peuchot
// This code is distributed under GNU GPL v3 license

// ================= Commands buffer =================

// Commands are buffered
// so the i2c interruption handler can return quickly after append command
// commands are executed sequentially by loop function

const int MAX_COMMANDS_COUNT = 5;
String commands[MAX_COMMANDS_COUNT];
int command_count = 0;
int current_command_index = -1; // -1 means "no running command"

void clearCommandBuffer()
{
    command_count = 0;
    current_command_index = -1;
}

void appendCommand(String command)
{
    if (command_count == MAX_COMMANDS_COUNT) {
        Serial.println("command buffer is full");
        return;
    }

    commands[command_count] = command;
    command_count++;
}

bool isCommandAvailable()
{
    return (current_command_index + 1 < command_count);
}

// return next command in the buffer or empty string is no more command
String getNextCommand()
{
    if (!isCommandAvailable()) {
        return "";
    }
    current_command_index++;
    return commands[current_command_index];
}

void printCommandBuffer()
{
    for (int i = 0; i < command_count; i++) {
        Serial.print("  commands[");
        Serial.print(i);
        Serial.print("]:");
        Serial.println(commands[i]);
    }
}
