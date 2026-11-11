#include <shared.h>
#include "classes.h"

void Scheduler::addTask(void (*fn)(), unsigned long interval)
{
    if(count< taskSize){
        tasks[count++] = {fn, interval, 0};
    }
}

void Scheduler::run()
{
    unsigned long now = millis();
    for (uint8_t i = 0; i < count; i++)
    {
        if(now - tasks[i].lastRun >= tasks[i].interval){
            tasks[i].lastRun = now;
            tasks[i].fn();
        }
    }
    
}


bool Scheduler::intervalBool(unsigned long interval, int id)
{
    unsigned long now = millis(); 
    if(now - intervalsLastMs[id] >= interval){
        intervalsLastMs[id] = now;
        return true; 
    }else{
        intervalsLastMs[id] = now;
        return false; 
    }
}
