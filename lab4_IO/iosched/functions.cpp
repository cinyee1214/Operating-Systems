#include <stdio.h>
#include <cmath>
#include <climits>
#include "classes.h"

using namespace std;

// IoOperation
IoOperation::IoOperation(int id, int timestamp, int track) {
  this->id = id;
  this->timestamp = timestamp;
  this->track = track;
}

// Scheduler
Scheduler::Scheduler() {
}

void Scheduler::addOperation(IoOperation* io) {
  this->IoQueue.push_back(io);
}

IoOperation* Scheduler::getNextOpertation(int currentTrack) {
  if (IoQueue.empty()) return nullptr;

  IoOperation* io = IoQueue.front();
  IoQueue.pop_front();
  return io;
}

// FIFO
FIFO::FIFO() {
}

// SSTF
SSTF::SSTF() {
}

IoOperation* SSTF::getNextOpertation(int currentTrack) {
  if (IoQueue.empty()) return nullptr;

  int minSeek = INT_MAX;
  list<IoOperation*>::iterator itr, minItr; 
  for (itr = IoQueue.begin(); itr != IoQueue.end(); ++itr) {
    if (abs((*itr)->track - currentTrack) < minSeek) {
      minSeek = abs((*itr)->track - currentTrack);
      minItr = itr;
    }
  }
  IoOperation* res = (*minItr);
  IoQueue.erase(minItr);
  return res;
}

// LOOK
LOOK::LOOK(){
  this->up = true;
}

IoOperation* LOOK::getNextOpertation(int currentTrack) {
  if (IoQueue.empty()) return nullptr;

  int minSeek = INT_MAX;
  list<IoOperation*>::iterator itr, minItr; 
  for (itr = IoQueue.begin(); itr != IoQueue.end(); ++itr) {
    if (up) {
      if ((*itr)->track - currentTrack >= 0 && (*itr)->track - currentTrack < minSeek) {
        minSeek = (*itr)->track - currentTrack;
        minItr = itr;
      }
    } else {
      if ((*itr)->track - currentTrack <= 0 && currentTrack - (*itr)->track < minSeek) {
        minSeek = currentTrack - (*itr)->track;
        minItr = itr;
      }
    }
  }

  if (minSeek == INT_MAX) {
    up = !up;
    for (itr = IoQueue.begin(); itr != IoQueue.end(); ++itr) {
      if (up) {
        if ((*itr)->track - currentTrack >= 0 && (*itr)->track - currentTrack < minSeek) {
          minSeek = (*itr)->track - currentTrack;
          minItr = itr;
        }
      } else {
        if ((*itr)->track - currentTrack <= 0 && currentTrack - (*itr)->track < minSeek) {
          minSeek = currentTrack - (*itr)->track;
          minItr = itr;
        }
      }
    }
  }

  IoOperation* res = (*minItr);
  IoQueue.erase(minItr);
  return res;
}

// CLOOK
CLOOK::CLOOK(){
}

IoOperation* CLOOK::getNextOpertation(int currentTrack) {
  if (IoQueue.empty()) return nullptr;

  int minTrack = INT_MAX;
  int minSeek = INT_MAX;
  list<IoOperation*>::iterator itr, minItr, minTrackItr; 
  for (itr = IoQueue.begin(); itr != IoQueue.end(); ++itr) {
    if ((*itr)->track - currentTrack >= 0 && (*itr)->track - currentTrack < minSeek) {
      minSeek = (*itr)->track - currentTrack;
      minItr = itr;
    }
    
    if ((*itr)->track < minTrack) {
      minTrack = (*itr)->track;
      minTrackItr = itr;
    }
  }

  if (minSeek == INT_MAX) {
    minItr = minTrackItr;
  }

  IoOperation* res = (*minItr);
  IoQueue.erase(minItr);
  return res;
}

// FLOOK
FLOOK::FLOOK(){
  this->up = true;
}

IoOperation* FLOOK::getNextOpertation(int currentTrack) {
  if (active_queue.empty()) {
    list<IoOperation*> tmp = active_queue;
    active_queue = IoQueue;
    IoQueue = tmp;
  }

  if (active_queue.empty()) return nullptr;

  int minSeek = INT_MAX;
  list<IoOperation*>::iterator itr, minItr; 
  for (itr = active_queue.begin(); itr != active_queue.end(); ++itr) {
    if (up) {
      if ((*itr)->track - currentTrack >= 0 && (*itr)->track - currentTrack < minSeek) {
        minSeek = (*itr)->track - currentTrack;
        minItr = itr;
      }
    } else {
      if ((*itr)->track - currentTrack <= 0 && currentTrack - (*itr)->track < minSeek) {
        minSeek = currentTrack - (*itr)->track;
        minItr = itr;
      }
    }
  }

  if (minSeek == INT_MAX) {
    up = !up;
    for (itr = active_queue.begin(); itr != active_queue.end(); ++itr) {
      if (up) {
        if ((*itr)->track - currentTrack >= 0 && (*itr)->track - currentTrack < minSeek) {
          minSeek = (*itr)->track - currentTrack;
          minItr = itr;
        }
      } else {
        if ((*itr)->track - currentTrack <= 0 && currentTrack - (*itr)->track < minSeek) {
          minSeek = currentTrack - (*itr)->track;
          minItr = itr;
        }
      }
    }
  }

  IoOperation* res = (*minItr);
  active_queue.erase(minItr);
  return res;
}