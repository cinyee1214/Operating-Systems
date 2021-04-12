#ifndef SCHEDULERS_H
#define SCHEDULERS_H
#include <iostream>
#include <list>
#include "classes.h"
using namespace std;

class Scheduler 
{ 
  public:
    Scheduler();

    list<Process*> runQueue;
    int quantum, maxprio;
    string scheName;
    int block_end;
    int non_io;
    bool showRunQ;

    virtual void add_process(Process *p);
    virtual Process* get_next_process();
    virtual void printRunQueue();
};

class FCFS: public Scheduler
{
  public:
    FCFS(bool showRunQ);
};

class LCFS: public Scheduler
{
  public:
    LCFS(bool showRunQ);
    void add_process(Process *p);
};

class SRTF: public Scheduler
{
  public:
    SRTF(bool showRunQ);
    void add_process(Process *p);
};

class RR: public Scheduler
{
  public:
    RR(bool showRunQ, int quantum);
};

class PRIO: public Scheduler
{
  public:
    PRIO(bool showRunQ, int quantum, int maxprio);

    list<Process*> *activeQ;
    list<Process*> *expiredQ;

    void add_process(Process *p);
    Process* get_next_process();
    void printRunQueue();
};

class PREPRIO: public Scheduler
{
  public:
    PREPRIO(bool showRunQ, int quantum, int maxprio);

    list<Process*> *activeQ;
    list<Process*> *expiredQ;

    void add_process(Process *p);
    Process* get_next_process();
    void printRunQueue();
};

#endif