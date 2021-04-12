#ifndef CLASSES_H
#define CLASSES_H
#include <iostream>
#include <list>
#include <map>
using namespace std;

typedef enum {CREATED, READY, RUNNG, BLOCK, PREEMPT} Process_State;
typedef enum {TRANS_TO_READY, TRANS_TO_RUN, TRANS_TO_BLOCK, TRANS_TO_PREEMPT} Transition;

class Process 
{
  public:
    Process(int pid, int AT, int TC, int CB, int IB, int static_prio);

    // processId, process arrival/start, Total CPU-time, CPU Burst, IO Burst, static_prio
    int pid, AT, TC, CB, IB, static_prio;
    int finish_time, turnarount_time, io_time, cpuWait_time;
    int state_timestamp;
    int cb, ib, rem;
    int dynamic_prio;
    Process_State state;
};

class Event {
  public:
    Event(int timestamp, Process* proc, Transition transition);

    int timestamp;
    Process* proc;
    Transition transition;
};

class DESlayer
{
  public:
    // constructor
    DESlayer(bool showEventQ, map<Transition, string> transitionMap);
    // attributes
    list<Event*> eventQueue;
    bool showEventQ;
    map<Transition, string> transitionMap;
    // functions
    void pushEvent(Event* event);
    Event* pollEvent();
    int get_next_event_time();
    void removeEvent(Process* p);
    void showEventQueue();
    void printEventQueue();
};

#endif