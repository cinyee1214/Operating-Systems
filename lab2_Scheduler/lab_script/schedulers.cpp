#include <string>
#include <stdio.h>
#include "schedulers.h"
#include "classes.h"

using namespace std;

Scheduler::Scheduler() {
  quantum = 10000;
  maxprio = 4;
  block_end = 0;
  non_io = 0;
}

void Scheduler::add_process(Process *p) {
  runQueue.push_back(p);
}

Process* Scheduler::get_next_process() {
  if (showRunQ) {
    printRunQueue();
  } 

  if (runQueue.empty()) return nullptr;
  Process *p = runQueue.front();
  runQueue.pop_front();
  return p;
}

void Scheduler::printRunQueue() {
  printf("SCHED(%lu): ", runQueue.size());
  list<Process*>::iterator itr; 
  for (itr = runQueue.begin(); itr != runQueue.end(); ++itr) {
    cout << (*itr)->pid << ":" << (*itr)->state_timestamp << " "; 
  }
  cout << endl;
}

// FCFS
FCFS::FCFS(bool showRunQ) {
  scheName = "FCFS";
  this->showRunQ = showRunQ;
}

// LCFS
LCFS::LCFS(bool showRunQ) {
  scheName = "LCFS";
  this->showRunQ = showRunQ;
}

void LCFS::add_process(Process *p) { 
  runQueue.push_front(p);
}

// SRTF
SRTF::SRTF(bool showRunQ) {
  scheName = "SRTF";
  this->showRunQ = showRunQ;
}

void SRTF::add_process(Process *p) {
  list<Process*>::iterator itr; 
  for (itr = runQueue.begin(); itr != runQueue.end(); ++itr) {
    if((*itr)->rem > p->rem) {
      break;
    }
  }
  runQueue.insert(itr, p);
}

// RR
RR::RR(bool showRunQ, int quantum) {
  this->quantum = quantum;
  this->showRunQ = showRunQ;
  scheName = "RR " + to_string(quantum);
}

// PRIO
PRIO::PRIO(bool showRunQ, int quantum, int maxprio) {
  this->quantum = quantum;
  this->showRunQ = showRunQ;
  scheName = "PRIO " + to_string(quantum);
  this->maxprio = maxprio;

  activeQ = new list<Process*>[maxprio];
  expiredQ = new list<Process*>[maxprio];
}

void PRIO::add_process(Process *p) {
  if (p->dynamic_prio == -1) {
    p->dynamic_prio = p->static_prio - 1;
    expiredQ[maxprio - 1 - p->dynamic_prio].push_back(p);
  } else {
    activeQ[maxprio - 1 - p->dynamic_prio].push_back(p);
  }
}

Process* PRIO::get_next_process() {
  if (showRunQ) {
    printRunQueue();
  } 

  int j = -1;
  for (int i = 0; i < maxprio; ++i) {
    if (!activeQ[i].empty()) {
      j = i;
      break;
    }
  }
  if (j == -1) {
    list<Process*>* tmp = activeQ;
    activeQ = expiredQ;
    expiredQ = tmp;
    for (int i = 0; i < maxprio; ++i) {
      if (!activeQ[i].empty()) {
        j = i;
        break;
      }
    }
  }
  if (j == -1) return nullptr;
  
  Process *p = activeQ[j].front();
  activeQ[j].pop_front();
  return p;
}

void PRIO::printRunQueue() {
  cout << "{";
  for (int i = 0; i < maxprio; ++i) {
    cout << "[";
    if (!activeQ[i].empty()) {
      list<Process*>::iterator itr; 
      for (itr = activeQ[i].begin(); itr != activeQ[i].end(); ++itr) {
        cout << (*itr)->pid << ","; 
      }
    }
    cout << "]";
  }

  cout << ":";
  for (int i = 0; i < maxprio; ++i) {
    cout << "[";
    if (!expiredQ[i].empty()) {
      list<Process*>::iterator itr; 
      for (itr = expiredQ[i].begin(); itr != expiredQ[i].end(); ++itr) {
        cout << (*itr)->pid << ","; 
      }
    }
    cout << "]";
  }
  cout << "}" << endl;
}
// PREPRIO
PREPRIO::PREPRIO(bool showRunQ, int quantum, int maxprio) {
  this->quantum = quantum;
  this->showRunQ = showRunQ;
  scheName = "PREPRIO " + to_string(quantum);
  this->maxprio = maxprio;

  activeQ = new list<Process*>[maxprio];
  expiredQ = new list<Process*>[maxprio];
}

void PREPRIO::add_process(Process *p) {
  if (p->dynamic_prio == -1) {
    p->dynamic_prio = p->static_prio - 1;
    expiredQ[maxprio - 1 - p->dynamic_prio].push_back(p);
  } else {
    activeQ[maxprio - 1 - p->dynamic_prio].push_back(p);
  }
}

Process* PREPRIO::get_next_process() {
  if (showRunQ) {
    printRunQueue();
  } 

  int j = -1;
  for (int i = 0; i < maxprio; ++i) {
    if (!activeQ[i].empty()) {
      j = i;
      break;
    }
  }
  if (j == -1) {
    list<Process*>* tmp = activeQ;
    activeQ = expiredQ;
    expiredQ = tmp;
    for (int i = 0; i < maxprio; ++i) {
      if (!activeQ[i].empty()) {
        j = i;
        break;
      }
    }
  }
  if (j == -1) return nullptr;
  
  Process *p = activeQ[j].front();
  activeQ[j].pop_front();
  return p;
}

void PREPRIO::printRunQueue() {
  cout << "{";
  for (int i = 0; i < maxprio; ++i) {
    cout << "[";
    if (!activeQ[i].empty()) {
      list<Process*>::iterator itr; 
      for (itr = activeQ[i].begin(); itr != activeQ[i].end(); ++itr) {
        cout << (*itr)->pid << ","; 
      }
    }
    cout << "]";
  }

  cout << ":";
  for (int i = 0; i < maxprio; ++i) {
    cout << "[";
    if (!expiredQ[i].empty()) {
      list<Process*>::iterator itr; 
      for (itr = expiredQ[i].begin(); itr != expiredQ[i].end(); ++itr) {
        cout << (*itr)->pid << ","; 
      }
    }
    cout << "]";
  }
  cout << "}" << endl;
}