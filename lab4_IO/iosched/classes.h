#ifndef CLASSES_H
#define CLASSES_H
#include <iostream>
#include <list>
#include <map>
using namespace std;

class IoOperation {
  public:
    IoOperation(int id, int timestamp, int track);

    int id;
    int timestamp;
    int track;
    int start_time, end_time;
};

class Scheduler
{
  public:
    Scheduler();

    list<IoOperation*> IoQueue;

    virtual void addOperation(IoOperation* io);
    virtual IoOperation* getNextOpertation(int currentTrack);
};

class FIFO: public Scheduler
{
  public:
    FIFO();
};

class SSTF: public Scheduler
{
  public:
    SSTF();

    IoOperation* getNextOpertation(int currentTrack);
};

class LOOK: public Scheduler
{
  public:
    LOOK();

    bool up;

    IoOperation* getNextOpertation(int currentTrack);
};

class CLOOK: public Scheduler
{
  public:
    CLOOK();

    IoOperation* getNextOpertation(int currentTrack);
};

class FLOOK: public Scheduler
{
  public:
    FLOOK();

    list<IoOperation*> active_queue;
    bool up;

    IoOperation* getNextOpertation(int currentTrack);
};

#endif