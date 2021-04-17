#include <string>
#include <map>
#include <stdio.h>
#include "classes.h"

using namespace std;

// Process
Process::Process(int pid, int pagetable_size, int vma_num) {
  this->pid = pid;
  this->unmaps = 0; 
  this->maps = 0;
  this->ins = 0;
  this->outs = 0;
  this->fins = 0;
  this->fouts = 0;
  this->zeros = 0;
  this->segv = 0;
  this->segprot = 0;
  this->vma_num = vma_num;
  page_table = new PTE[pagetable_size];
  for (int i = 0; i < pagetable_size; ++i) {
    PTE pte;
    pte.FRAME = 0;
    pte.PRESENT = 0;
    pte.REFERENCED = 0;
    pte.MODIFIED = 0;
    pte.WRITE_PROTECT = 0;
    pte.PAGEDOUT = 0;
    pte.FILE_MAPPED = 0;
    pte.VMA_CHECKED = 0;

    page_table[i] = pte;
  }
  vmas = new VMA[vma_num];
}

// Pager
Pager::Pager() {
  this->counter = 0;
  this->lastReSet = -1;
}

void Pager::addFrame(Frame* frame) {
  if (frame->beCandidate) {
    return;
  }
  frame->beCandidate = true;
  candidates.push_back(frame);
}

Frame* Pager::select_victim_frame() {
  Frame* frame = candidates.front();
  candidates.erase(candidates.begin());
  frame->beCandidate = false;
  return frame;
}

FIFO::FIFO() {
}

Clock::Clock() {
}

void Clock::addFrame(Frame* frame) {
  if (frame->beCandidate) {
    return;
  }
  frame->beCandidate = true;
  candidates.insert(candidates.begin()+counter, frame);
  counter++;
  counter = counter == tail ? 0 : counter;
}

Frame* Clock::select_victim_frame() {
  PTE* pte = candidates[counter]->pte;
  while (pte->REFERENCED) {
    pte->REFERENCED = 0;
    counter++;
    counter = counter == tail ? 0 : counter;
    pte = candidates[counter]->pte;
  }
  Frame* frame = candidates[counter];
  candidates.erase(candidates.begin()+counter);
  frame->beCandidate = false;
  return frame;
}

ESC::ESC() {
  classes = new int[4];
  for (int i = 0; i < 4; ++i) {
    classes[i] = -1;
  }
}

void ESC::addFrame(Frame* frame) {
  if (frame->beCandidate) {
    return;
  }
  frame->beCandidate = true;
  candidates.insert(candidates.begin()+counter, frame);
  counter++;
  counter = counter == tail ? 0 : counter;
}

Frame* ESC::select_victim_frame() {
  for (int i = 0; i < candidates.size(); ++i) {
    int class_index = candidates[counter]->pte->REFERENCED * 2 + candidates[counter]->pte->MODIFIED;
    if (classes[class_index] == -1) {
      classes[class_index] = counter;
    }
    if (class_index == 0) break;
    counter++;
    counter = counter == tail ? 0 : counter;
  }

  int cur = -1;
  for (int i = 0; i < 4; ++i) {
    if (classes[i] == -1) {
      continue;
    }
    if (cur == -1) cur = classes[i];
    classes[i] = -1;
  }
  counter = cur;
  counter = counter == tail ? 0 : counter;
  Frame* frame = candidates[cur];
  candidates.erase(candidates.begin()+cur);
  frame->beCandidate = false;

  if (instr - lastReSet >= 50) {
    lastReSet = instr;
    for (int i = 0; i < candidates.size(); ++i) {
      candidates[i]->pte->REFERENCED = 0;
    }
  }
  return frame;
}

Aging::Aging() {
}

void Aging::addFrame(Frame* frame) {
  if (candidates.size() == tail) {
    return;
  }
  candidates.push_back(frame);
}

void Aging::setAge(Frame* frame) {
  frame->age >>= 1;
  if (frame->pte->REFERENCED) {
    frame->age = (frame->age | 0x80000000);
    frame->pte->REFERENCED = 0;
  } 
}

Frame* Aging::select_victim_frame() {
  Frame* frame;
  unsigned int cur = 0xffffffff;
  int curCounter = -1;
  for (int i = 0; i < candidates.size(); ++i) { 
    setAge(candidates[counter]);
    if (candidates[counter]->age < cur) {
      cur = candidates[counter]->age;
      frame = candidates[counter];
      curCounter = counter;
    }
    counter++;
    counter = counter == tail ? 0 : counter;
  }
  counter = curCounter + 1;
  counter = counter == tail ? 0 : counter;

  return frame;
}

WorkingSet::WorkingSet() {
}

void WorkingSet::addFrame(Frame* frame) {
  if (frame->beCandidate) {
    return;
  }
  frame->beCandidate = true;
  candidates.insert(candidates.begin()+counter, frame);
  counter++;
  counter = counter == tail ? 0 : counter;
}

Frame* WorkingSet::select_victim_frame() {
  int cur = -1;
  int minAge = instr + 1;
  for (int i = 0; i < candidates.size() || cur == -1; ++i) {
    if (candidates[counter]->pte->REFERENCED) {
      candidates[counter]->pte->REFERENCED = 0;
      candidates[counter]->lastRef = instr;
    } else if (instr - candidates[counter]->lastRef >= 50) {
      cur = counter;
      break;
    } else if (minAge > candidates[counter]->lastRef) {
      minAge = candidates[counter]->lastRef;
      cur = counter;
    }
    counter++;
    counter = counter == tail ? 0 : counter;
  }

  counter = cur;
  counter = counter == tail ? 0 : counter;
  Frame* frame = candidates[cur];
  candidates.erase(candidates.begin()+cur);
  frame->beCandidate = false;
  return frame;
}


