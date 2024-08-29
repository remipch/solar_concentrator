// Copyright (C) 2024 Rémi Peuchot
// This code is distributed under GNU GPL v3 license (see software/LICENSE.md)

#include <SoftwareSerial.h>

const int MOTOR_PINS_COUNT = 8;

// Motor pins ordered by motors command bit (loweest bit at index 0)
int MOTOR_PINS[MOTOR_PINS_COUNT] = {
    // PANEL_0:
    2, //  LEFT motor, pin A (unroll)   1
    3, //  LEFT motor, pin B (roll)     2
    4, //  RIGHT motor, pin A (unroll)  4
    5, //  RIGHT motor, pin B (roll)    8
    // PANEL_1:
    7, // LEFT motor, pin A (unroll)   16
    10, // LEFT motor, pin B (roll)     32
    11, // RIGHT motor, pin A (unroll)  64
    12, // RIGHT motor, pin B (roll)    128
};

int PWM_PIN = 6; // dedicated to motors speed

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
    for (int i = 0; i < MOTOR_PINS_COUNT; i++) {
        pinMode(MOTOR_PINS[i], OUTPUT);
    }
    pinMode(PWM_PIN, OUTPUT);
    analogWrite(PWM_PIN, 255);

    printRam();
    printSampling();
    stopAndClearCommands();
}

void loop()
{
    String commands = "";
    if (Serial.available() > 0) {
        commands = Serial.readStringUntil('\n');
    } else if (MasterSerial.available() > 0) {
        commands = MasterSerial.readStringUntil('\n');
    }
    if (commands.length() > 0) {
        Serial.print("parse commands: ");
        Serial.println(commands);
        parseAndRunCommands(commands);
    }

    // Start/update/terminate commands depending on command buffer and current command state
    run();
}

void printRam()
{
    extern unsigned int __data_start;
    extern unsigned int __bss_start;
    extern unsigned int __heap_start;
    extern void* __brkval;
    byte stack_start;
    unsigned int heap_end = (__brkval == 0 ? (unsigned int)&__heap_start : (unsigned int)__brkval);
    unsigned int stackSize = RAMEND - (unsigned int)&stack_start + 1;
    unsigned int freeMem = ((unsigned int)&stack_start) - heap_end;

    Serial.print(F("data : "));
    Serial.print((unsigned int)&__bss_start - (unsigned int)&__data_start);
    Serial.print(F(" byte(s) from 0x"));
    Serial.println((unsigned int)&__data_start, 16);

    Serial.print(F("bss : "));
    Serial.print((unsigned int)&__heap_start - (unsigned int)&__bss_start);
    Serial.print(F(" byte(s) from 0x"));
    Serial.println((unsigned int)&__bss_start, 16);

    Serial.print(F("heap : "));
    Serial.print(heap_end - (unsigned int)&__heap_start);
    Serial.print(F(" byte(s) from 0x"));
    Serial.println((unsigned int)&__heap_start, 16);

    Serial.print(F("stack : "));
    Serial.print(stackSize);
    Serial.print(F(" byte(s) from 0x"));
    Serial.println((unsigned int)&stack_start, 16);

    Serial.print(F("free RAM : "));
    Serial.print(freeMem);
    Serial.println(F(" bytes"));
    Serial.println();
}
