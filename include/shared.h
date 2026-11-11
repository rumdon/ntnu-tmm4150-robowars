#pragma once
#include <Arduino.h>


constexpr uint8_t DC_M = 0;
constexpr uint8_t SERVO_M = 1;


#define INTERVAL(ms) ([]{ \
    static unsigned long last = 0; \
    unsigned long now = millis(); \
    if (now - last >= (ms)) { \
         last = now; return true; } \
    return false; }())   

