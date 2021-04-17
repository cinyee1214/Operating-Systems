#ifndef CLASSES_H
#define CLASSES_H
#include <iostream>
#include <vector>
#include <map>
using namespace std;

typedef struct { 
  unsigned FRAME:7;
	unsigned PRESENT:1;
	unsigned REFERENCED:1;
	unsigned MODIFIED:1;
	unsigned WRITE_PROTECT:1;
  unsigned PAGEDOUT:1;
  unsigned FILE_MAPPED:1;
  unsigned VMA_CHECKED:2;

	unsigned unused:17;
} PTE; // can only be total of 32-bit size

typedef struct {
  int fid;
  int pid;
  int vpage;
  bool beCandidate;
  int lastRef;
  unsigned int age;
  PTE* pte;
} Frame;

typedef struct {
  int start_vpage;
  int end_vpage;
  bool write_protected;
  bool file_mapped;
} VMA;

typedef struct {
  char instruction;
  int vpage;
} Instruction;

class Process 
{
  public:
    Process(int pid, int pagetable_size, int vma_num);

    int pid, vma_num;
    unsigned long unmaps, maps, ins, outs, fins, fouts, zeros, segv, segprot;
    PTE* page_table;
    VMA* vmas;
};

class Pager 
{
  public:
    Pager();

    int counter;
    int tail;
    int lastReSet;
    int instr;
    vector<Frame*> candidates;
    
    virtual void addFrame(Frame* frame);
    virtual Frame* select_victim_frame(); 
};

class FIFO: public Pager
{
  public:
    FIFO();
};

class Clock: public Pager
{
  public:
    Clock();

    void addFrame(Frame* frame);
    Frame* select_victim_frame();
};

class ESC: public Pager
{
  public:
    ESC();

    int* classes;

    void addFrame(Frame* frame);
    Frame* select_victim_frame();
};

class Aging: public Pager
{
  public:
    Aging();

    void addFrame(Frame* frame);
    void setAge(Frame* frame);
    Frame* select_victim_frame(); 
};

class WorkingSet: public Pager
{
  public:
    WorkingSet();

    void addFrame(Frame* frame);
    Frame* select_victim_frame();
};

#endif