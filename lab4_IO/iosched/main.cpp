#include <cstring>
#include <sstream>
#include <fstream>
#include <cctype>
#include <iterator> 
#include <cmath>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <cstdlib>
#include "classes.h"

using namespace std;

ifstream inFile;
char* inputFile;
string algo = "i";
Scheduler* sched;
bool vFlag = false;
bool qFlag = false;
bool fFlag = false;
IoOperation* curOperation = nullptr;
list<IoOperation*> operations;
list<IoOperation*>::iterator currentItr;
int currentTrack = 0;
int current_direction = 0;
int totMovement = 0;
int maxWaittime = 0;

void parseArguments(int argc, char* argv[]);
void readInputFile(char* inputFile);
void processOperations();
void printFinalSummary();

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "Not a valid argument" << endl;
    exit(EXIT_FAILURE);
  }

  parseArguments(argc, argv);
  readInputFile(inputFile);
  processOperations();
  printFinalSummary();
  return 0;
}

void parseArguments(int argc, char* argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "s:vqf")) != -1){
    switch (opt){
      case 's':
      {
        algo = optarg;
        break;
      }
      case 'v':
      {  
        vFlag = true;
        break;
      }
      case 'q':
      {
        qFlag = true;
        break;
      }
      case 'f':
      { 
        fFlag = true; 
        break;
      }
      default:
      {  
        break;
      }
    }
  }

  inputFile = argv[optind++];

  switch (algo[0]) {
    case 'i':
    { 
      sched = new FIFO();
      break;
    }
    case 'j':
    {
      sched = new SSTF();
      break;
    }
    case 's':
    {
      sched = new LOOK();
      break;
    }
    case 'c':
    {
      sched = new CLOOK();
      break;
    }
    case 'f':
    {
      sched = new FLOOK();
      break;
    }
  }
}

void readInputFile(char* inputFile) {
  inFile.open(inputFile);
  string curLine;
  int id = 0;
  string timestamp, track;
  while (getline(inFile, curLine)) {
    if (curLine[0] == '#') continue;
    
    istringstream streamLine(curLine);
    streamLine >> timestamp >> track;
    
    IoOperation* curIO = new IoOperation(id, stoi(timestamp), stoi(track));
    operations.push_back(curIO);
    id++;
  }

  inFile.close();
}

void processOperations() {
  currentItr = operations.begin();
  int currentTime = 0;
  while (curOperation != nullptr || currentItr != operations.end()) {
    if (currentItr != operations.end() && (*currentItr)->timestamp == currentTime) {
      sched->addOperation(*currentItr);
      currentItr++;
    }

    if (curOperation != nullptr) {
      if (curOperation->end_time == currentTime) {
        curOperation = nullptr;
      } else {
        currentTrack += current_direction;
        currentTime++;
      }
    } 

    if (curOperation == nullptr) {
      curOperation = sched->getNextOpertation(currentTrack);
      if (curOperation == nullptr) {
        currentTime++;
        continue;
      }

      maxWaittime = max(maxWaittime, currentTime - curOperation->timestamp);
      curOperation->start_time = currentTime;
      int current_move = abs(curOperation->track - currentTrack);
      totMovement += current_move;
      curOperation->end_time = curOperation->start_time + current_move;
      if (curOperation->track == currentTrack) {
        current_direction = 0;
      } else if (curOperation->track > currentTrack) {
        current_direction = 1;
      } else {
        current_direction = -1;
      }
    }
  }
}

void printFinalSummary() {
  int count = 0;
  int totalTime = 0;
  double totalTurn = 0;
  double totalWait = 0;

  while (!operations.empty()) {
    IoOperation* cur = operations.front();
    operations.pop_front();

    totalTime = max(totalTime, cur->end_time);
    totalTurn += cur->end_time - cur->timestamp;
    totalWait += cur->start_time - cur->timestamp;
    count++;

    printf("%5d: %5d %5d %5d\n", cur->id, cur->timestamp, cur->start_time, cur->end_time);
  }
  printf("SUM: %d %d %.2lf %.2lf %d\n",
          totalTime, totMovement, totalTurn/count, totalWait/count, maxWaittime);
}