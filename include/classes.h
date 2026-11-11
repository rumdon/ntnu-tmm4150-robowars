#pragma once
#include <Arduino.h>

#ifndef SCHEDULER_SIZE
#define SCHEDULER_SIZE 32
#endif

#ifndef INTERVAL_SIZE
#define INTERVAL_SIZE 32
#endif

class Scheduler {
    public:
        void addTask(void(*fn)(),unsigned long interval);
        void run();
        bool intervalBool(unsigned long interval, int id);
    private:
        struct Task {
            void (*fn)();
            unsigned long interval;
            unsigned long lastRun;
        };
        const static uint8_t intervalSize = INTERVAL_SIZE;
        unsigned long intervalsLastMs[intervalSize] = {};


        const static uint8_t taskSize = SCHEDULER_SIZE;
        uint8_t count = 0;
        Task tasks[taskSize];
};

extern Scheduler scheduler;

