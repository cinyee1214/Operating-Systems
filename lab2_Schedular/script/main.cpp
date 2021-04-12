#include <cstring>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cctype>
#include <iterator> 
#include <algorithm>
#include <cmath>
#include <climits>
#include <queue>
#include <map>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "schedulers.h"
#include "classes.h"

using namespace std;

int myrandom(int burst);
void parseArguments(int argc, char* argv[]);
void readInputFile(DESlayer* DES_Layer, char* inputFile);
void readRandFile(char* randFile);
void simulate(DESlayer* DES_Layer, Scheduler* sched);
void initializeMaps();
void showEvent(Event* evt, int timeInPrevState, Process_State prev_state);
void printFinalSummary();

bool verbose = false; //-v
bool showEventQ = false; //-v -e
bool showRunQ = false; //-v -t
string schedSpec = "F";
char* inputFile;
char* randFile;
queue<Process*> procQueue;
Scheduler* sched;
int maxprio = 4;
int quantum;
vector<int> randvals;
int ofs = 0;
int counts;
ifstream inFile;
int CURRENT_TIME = 0;
bool CALL_SCHEDULER = false;
Process* CURRENT_RUNNING_PROCESS = nullptr;
map<Process_State, string> procStateMap;
map<Transition, string> transitionMap;
bool isE = false;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Not a valid argument" << endl;
    exit(EXIT_FAILURE);
  }

  initializeMaps();

  parseArguments(argc, argv);
  showEventQ = showEventQ && verbose;
  showRunQ = showRunQ && verbose;

  readRandFile(randFile);

  DESlayer* DES_Layer = new DESlayer(showEventQ, transitionMap);
  readInputFile(DES_Layer, inputFile);
  
  simulate(DES_Layer, sched);
  
  printFinalSummary();
  return 0;
}

void initializeMaps() {
  // CREATED, READY, RUNNG, BLOCK, PREEMPT
  procStateMap.insert(pair<Process_State, string>(CREATED, "CREATED"));
  procStateMap.insert(pair<Process_State, string>(READY, "READY"));
  procStateMap.insert(pair<Process_State, string>(RUNNG, "RUNNG"));
  procStateMap.insert(pair<Process_State, string>(BLOCK, "BLOCK"));
  procStateMap.insert(pair<Process_State, string>(PREEMPT, "READY"));
  // TRANS_TO_READY, TRANS_TO_RUN, TRANS_TO_BLOCK, TRANS_TO_PREEMPT
  transitionMap.insert(pair<Transition, string>(TRANS_TO_READY, "READY"));
  transitionMap.insert(pair<Transition, string>(TRANS_TO_RUN, "RUNNG"));
  transitionMap.insert(pair<Transition, string>(TRANS_TO_BLOCK, "BLOCK"));
  transitionMap.insert(pair<Transition, string>(TRANS_TO_PREEMPT, "PREEMPT"));
}

int myrandom(int burst) {
  ofs %= counts;
  return 1 + (randvals[ofs++] % burst);
}

void parseArguments(int argc, char* argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "vtes:")) != -1){
    switch (opt){
      case 'v':
      {  
        verbose = true;
        break;
      }
      case 't':
      {
        showRunQ = true;
        break;
      }
      case 'e':
      { 
        showEventQ = true; 
        break;
      }
      case 's':
      {
        schedSpec = optarg;
        break;
      }
      default:
      {  
        break;
      }
    }
  }

  inputFile = argv[optind++];
  randFile = argv[optind++];

  switch (schedSpec[0]) { 
    case 'F':
    {
      sched = new FCFS(showRunQ);
      break;
    }
    case 'L':
    {
      sched = new LCFS(showRunQ);
      break;
    }   
    case 'S':
    {
      sched = new SRTF(showRunQ);
      break;
    }
    case 'R':
    { 
      quantum = stoi(schedSpec.substr(1));
      sched = new RR(showRunQ, quantum);
      break;
    }
    case 'P':
    {
      size_t colon_pos = schedSpec.find(':');
      if (colon_pos != -1) {
        maxprio = stoi(schedSpec.substr(colon_pos+1));
        quantum = stoi(schedSpec.substr(1, colon_pos-1));
      } else {
        quantum = stoi(schedSpec.substr(1));
      }
      sched = new PRIO(showRunQ, quantum, maxprio);
      break;
    }
    default:  // 'E'
    {
      size_t colon_pos = schedSpec.find(':');
      if (colon_pos != -1) {
        maxprio = stoi(schedSpec.substr(colon_pos+1));
        quantum = stoi(schedSpec.substr(1, colon_pos-1));
      } else {
        quantum = stoi(schedSpec.substr(1));
      }
      sched = new PREPRIO(showRunQ, quantum, maxprio);
      isE = true;
      break;
    }
  }
}

void readInputFile(DESlayer* DES_Layer, char* inputFile) {
  inFile.open(inputFile);
  string curLine;
  int pid = 0;
  int AT, TC, CB, IB;
  while (getline(inFile, curLine)) {
    istringstream streamLine(curLine);
    streamLine >> AT >> TC >> CB >> IB;
    int static_prio = myrandom(maxprio);
    Process* process = new Process(pid, AT, TC, CB, IB, static_prio);
    procQueue.push(process);

    Event* event = new Event(AT, process, TRANS_TO_READY);
    DES_Layer->eventQueue.push_back(event);
    pid++;
  }
  inFile.close();
}

void readRandFile(char* randFile) {
  inFile.open(randFile);
  string curLine;
  int curNum;
  
  //get counts
  if (getline(inFile, curLine)) {
    istringstream streamLine(curLine);
    streamLine >> counts;
  }
  //push to randvals
  while (getline(inFile, curLine)) {
    istringstream streamLine(curLine);
    streamLine >> curNum;
    randvals.push_back(curNum);
  }
  inFile.close();
}

void simulate(DESlayer* DES_Layer, Scheduler* sched) {
  if (showEventQ) {
    cout << "ShowEventQ: ";
    DES_Layer->showEventQueue();
    cout << endl;
  }
  
  Event* evt;
  int timeInPrevState;

  while((evt = DES_Layer->pollEvent())) {
    Process *proc = evt->proc;
    CURRENT_TIME = evt->timestamp;
    timeInPrevState = CURRENT_TIME - proc->state_timestamp;
    Process_State prev_state = proc->state;

    switch(evt->transition) { // which state to transition to?
      case TRANS_TO_READY:
      {
        proc->state = READY;
        proc->state_timestamp = CURRENT_TIME;
        // must come from BLOCKED(4)/created(1) or from PREEMPTION
        if (prev_state == BLOCK) {
          proc->io_time += proc->ib;
          proc->dynamic_prio = proc->static_prio - 1;
        }

        if (verbose) {
          showEvent(evt, timeInPrevState, prev_state);
        }

        // pre-emption prio
        if (isE && CURRENT_RUNNING_PROCESS != nullptr) {
          if (CURRENT_RUNNING_PROCESS->state_timestamp + 
                min(CURRENT_RUNNING_PROCESS->cb, sched->quantum) > CURRENT_TIME 
              && proc->dynamic_prio > CURRENT_RUNNING_PROCESS->dynamic_prio) {
            DES_Layer->removeEvent(CURRENT_RUNNING_PROCESS);

            Event* event = new Event(CURRENT_TIME, CURRENT_RUNNING_PROCESS, TRANS_TO_PREEMPT);
            DES_Layer->pushEvent(event);
          }
        }
        // must add to run queue
        sched->add_process(proc);

        CALL_SCHEDULER = true; // conditional on whether something is run
        break;
      }
      case TRANS_TO_RUN:
      { 
        // create event for either preemption or blockings
        proc->state = RUNNG;
        proc->state_timestamp = CURRENT_TIME;
        CURRENT_RUNNING_PROCESS = proc;
        // On the same process: Termination takes precedence over scheduling the next IO burst 
        // over preempting the process on Quantum expiration.
        if (prev_state == READY) {
          proc->cb = myrandom(proc->CB);
          proc->cb = min(proc->cb, proc->rem);
        }
        
        if (verbose) {
          showEvent(evt, timeInPrevState, prev_state);
        }

        // ->block
        Event* event = new Event(CURRENT_TIME+proc->cb, proc, TRANS_TO_BLOCK);
        // ->preemption
        if (proc->cb > sched->quantum) {
          event->timestamp = CURRENT_TIME+sched->quantum;
          event->transition = TRANS_TO_PREEMPT;
        }
        // push event to eventQ
        DES_Layer->pushEvent(event);
        break;
      }
      case TRANS_TO_BLOCK:
      {
        proc->state = BLOCK;
        proc->state_timestamp = CURRENT_TIME;
        proc->rem -= proc->cb;
        proc->cb = 0;
        CURRENT_RUNNING_PROCESS = nullptr;

        // On the same process: Termination takes precedence over scheduling the next IO burst 
        // termination
        if(proc->rem == 0){
          proc->finish_time = CURRENT_TIME;
          if (verbose) printf("%d %d %d: Done \n", CURRENT_TIME, proc->pid, timeInPrevState);
          CALL_SCHEDULER = true;
          break;
        }
        //create an event for when process becomes READY again
        proc->ib = myrandom(proc->IB);

        if (CURRENT_TIME > sched->block_end) {
          sched->non_io += CURRENT_TIME - sched->block_end;
        }
        sched->block_end = max(sched->block_end, CURRENT_TIME + proc->ib);

        if (verbose) {
          showEvent(evt, timeInPrevState, prev_state);
        }

        Event* event = new Event(CURRENT_TIME+proc->ib, proc, TRANS_TO_READY);
        // push event to eventQ
        DES_Layer->pushEvent(event);

        CALL_SCHEDULER = true;
        break;
      }
      case TRANS_TO_PREEMPT:
      { 
        // run->ready
        CURRENT_RUNNING_PROCESS = nullptr;

        proc->state = PREEMPT;
        proc->state_timestamp = CURRENT_TIME;
        // proc->rem -= sched->quantum;
        // proc->cb -= sched->quantum;
        proc->rem -= timeInPrevState;
        proc->cb -= timeInPrevState;
         
        if (verbose) {
          showEvent(evt, timeInPrevState, prev_state);
        }

        proc->dynamic_prio--;

        // add to runqueue (no event is generated)
        sched->add_process(proc);

        CALL_SCHEDULER = true;
        break;
      }
    }
    // remove current event object from Memory
    delete evt; 
    evt = nullptr;

    if(CALL_SCHEDULER) {
      if (DES_Layer->get_next_event_time() == CURRENT_TIME) continue; //process next event from Event queue
      
      CALL_SCHEDULER = false; // reset global flag
      if (CURRENT_RUNNING_PROCESS == nullptr) {
        CURRENT_RUNNING_PROCESS = sched->get_next_process();
        if (CURRENT_RUNNING_PROCESS == nullptr) continue;
        // create event to make this process runnable for same time.
        Event* event = new Event(CURRENT_TIME, CURRENT_RUNNING_PROCESS, TRANS_TO_RUN);
        DES_Layer->pushEvent(event);
      }
    } 
  }   
}

void showEvent(Event* evt, int timeInPrevState, Process_State prev_state) {
  // termination
  if (evt->proc->finish_time != -1) {
    printf("%d %d %d: Done \n", evt->timestamp, evt->proc->pid, timeInPrevState);
    return;
  }

  if (evt->transition == TRANS_TO_PREEMPT) {
    // preemption
    // RUNNG -> READY  cb=3 rem=95 prio=1
    printf("%d %d %d: RUNNG -> READY ", evt->timestamp, evt->proc->pid, timeInPrevState);
    printf("cb=%d rem=%d prio=%d", evt->proc->cb, evt->proc->rem, evt->proc->dynamic_prio);
  } else {
    printf("%d %d %d: %s -> %s ", evt->timestamp, evt->proc->pid, timeInPrevState, 
          procStateMap.find(prev_state)->second.c_str(), 
          transitionMap.find(evt->transition)->second.c_str());
  }

  // READY -> RUNNG cb=1 rem=2 prio=1
  if (evt->transition == TRANS_TO_RUN) {
    printf("cb=%d rem=%d prio=%d", evt->proc->cb, evt->proc->rem, evt->proc->dynamic_prio);
  }
  // RUNNG -> BLOCK  ib=9 rem=2
  if (prev_state == RUNNG && evt->transition == TRANS_TO_BLOCK) {
    printf("ib=%d rem=%d", evt->proc->ib, evt->proc->rem);
  }
  cout << endl;
}

void printFinalSummary() {
  int total_sim = 0;
  double total_cpu = 0;
  double total_io;
  double total_turn = 0;
  double total_wait = 0;
  int count = procQueue.size();

  cout << sched->scheName << endl;

  while (!procQueue.empty()) {
    Process* cur = procQueue.front();
    procQueue.pop();

    cur->turnarount_time = cur->finish_time - cur->AT;
    cur->cpuWait_time = cur->turnarount_time - cur->io_time - cur->TC;

    total_sim = max(total_sim, cur->finish_time);
    total_cpu += cur->TC;
    
    total_turn += cur->turnarount_time;
    total_wait += cur->cpuWait_time;

    printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
            cur->pid, cur->AT, cur->TC, cur->CB, cur->IB, cur->static_prio,
            cur->finish_time, cur->turnarount_time, cur->io_time, cur->cpuWait_time
          );
  }

  sched->non_io += total_sim - sched->block_end;
  total_io = total_sim - sched->non_io;

  printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
          total_sim, 100*total_cpu/total_sim, 100*total_io/total_sim, total_turn/count,
          total_wait/count, count/(total_sim/100.0)
        );
}