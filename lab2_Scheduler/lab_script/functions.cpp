#include <string>
#include <map>
#include <stdio.h>
#include "schedulers.h"
#include "classes.h"

using namespace std;

// Process
Process::Process(int pid, int AT, int TC, int CB, int IB, int static_prio) {
    this->pid = pid;
    this->AT = AT;
    this->TC = TC;
    this->CB = CB;
    this->IB = IB;
    this->static_prio = static_prio;
    io_time = 0;
    state_timestamp = AT;
    cb = 0;
    ib = 0;
    rem = TC;
    dynamic_prio = static_prio - 1;
    state = CREATED;
    finish_time = -1;
}

// Event
Event::Event(int timestamp, Process* proc, Transition transition) {
    this->timestamp = timestamp;
    this->proc = proc;
    this->transition = transition;
}

// DESLayer
DESlayer::DESlayer(bool showEventQ, map<Transition, string> transitionMap){
  this->showEventQ = showEventQ;
  this->transitionMap = transitionMap;
}

void DESlayer::pushEvent(Event* event) {
  if (showEventQ) {
    cout << "AddEvent(" << event->timestamp << ":" 
          << event->proc->pid << ":"
          << transitionMap.find(event->transition)->second << "): ";
    printEventQueue();
  }

  // push new event
  list<Event*>::iterator itr; 
  for (itr = eventQueue.begin(); itr != eventQueue.end(); ++itr) {
    if((*itr)->timestamp > event->timestamp) {
      break;
    }
  }
  eventQueue.insert(itr, event);
  
  if (showEventQ) {
    cout << "==> ";
    printEventQueue();
    cout << endl;
  }
}

Event* DESlayer::pollEvent() {
  Event* e = eventQueue.front();
  eventQueue.pop_front();
  return e;
}

int DESlayer::get_next_event_time() {
  if (eventQueue.empty()) return -1;
  return eventQueue.front()->timestamp;
}

void DESlayer::removeEvent(Process* p) {
  if (showEventQ) {
    cout << "RemoveEvent(" << p->pid << "): ";
    printEventQueue();
  }

  list<Event*>::iterator itr; 
  for (itr = eventQueue.begin(); itr != eventQueue.end(); ++itr) {
    if ((*itr)->proc->pid == p->pid) break;
  }
  eventQueue.erase(itr);

  if (showEventQ) {
    cout << "==> ";
    printEventQueue();
    cout << endl;
  }
}

void DESlayer::showEventQueue() {
  list<Event*>::iterator itr; 
  for (itr = eventQueue.begin(); itr != eventQueue.end(); ++itr) {
    cout << (*itr)->timestamp << ":" << (*itr)->proc->pid << " "; 
  }
}

void DESlayer::printEventQueue() {
  list<Event*>::iterator itr; 
  for (itr = eventQueue.begin(); itr != eventQueue.end(); ++itr) {
    cout << (*itr)->timestamp << ":" << (*itr)->proc->pid << ":" 
         << transitionMap.find((*itr)->transition)->second << " "; 
  }
}

