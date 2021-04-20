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
#include <deque>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <cstdlib>
#include <map>
#include "classes.h"

using namespace std;

const int PT_SIZE = 64;
Process* current_proc = nullptr;
queue<Frame*> free_pool;
Frame** frame_table;
int num_frames = 4;
string algo = "f";
string options;
char* inputFile;
char* randFile;
ifstream inFile;
vector<int> randvals;
int ofs = 0;
int counts;
Process** process_table;
int process_count;
vector<Instruction> instructions;
bool optionO = false;
bool optionP = false;
bool optionF = false;
bool optionS = false;
bool isRandom = false;
Pager* pager;
unsigned long inst_count = 0;
unsigned long ctx_switches = 0;
unsigned long process_exits = 0;

void parseArguments(int argc, char* argv[]);
int myrandom(int burst);
void readRandFile(char* randFile);
void readInputFile(char* inputFile);
Frame* getNextFrame();
void processInstructions();
void releaseFrames();
void printFinalSummary();

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Not a valid argument" << endl;
    exit(EXIT_FAILURE);
  }

  parseArguments(argc, argv);

  readRandFile(randFile);
  readInputFile(inputFile);

  processInstructions();

  printFinalSummary();
  return 0;
}

void parseArguments(int argc, char* argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "f:a:o:")) != -1){
    switch (opt) {
      case 'f':
      {  
        num_frames = stoi(optarg);
        break;
      }
      case 'a':
      { 
        algo = optarg; 
        break;
      }
      case 'o':
      {
        options = optarg;
        for (int i = 0; i < options.length(); ++i) {
          switch (options[i]) {
            case 'O':
            {
              optionO = true;
              break;
            }
            case 'P':
            {
              optionP = true;
              break;
            }
            case 'F':
            {
              optionF = true;
              break;
            }
            case 'S':
            {
              optionS = true;
              break;
            }
          }
        }
        break;
      }
      default:
      {  
        break;
      }
    }
  }

  frame_table = new Frame*[num_frames];
  for (int i = 0; i < num_frames; ++i) {
    Frame* frame = new Frame;
    frame->fid = i;
    frame->pid = -1;
    frame->vpage = -1;
    // Frame frame ={.fid = i, .pid=-1, .vpage=-1};
    free_pool.push(frame);
    frame_table[i] = frame;
  }
  
  switch (algo[0]) {
    case 'f':
    { 
      pager = new FIFO();
      break;
    }
    case 'r':
    {
      isRandom = true;
      break;
    }
    case 'c':
    {
      pager = new Clock();
      break;
    }
    case 'e':
    {
      pager = new ESC();
      break;
    }
    case 'a':
    {
      pager = new Aging();
      break;
    }
    case 'w':
    {
      pager = new WorkingSet();
      break;
    }
  }
        
  inputFile = argv[optind++];
  randFile = argv[optind++];
  if (!isRandom) pager->tail = num_frames;
}

int myrandom(int burst) {
  // ofs = ofs == counts ? 0 : ofs;
  ofs %= counts;
  return randvals[ofs++] % burst;
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

void readInputFile(char* inputFile) {
  inFile.open(inputFile);
  string curLine;
  while (getline(inFile, curLine)) {
    if (curLine[0] == '#') continue;

    istringstream streamLine(curLine);
    streamLine >> process_count;
    break;
  }
  process_table = new Process*[process_count];

  for (int pid = 0; pid < process_count; ++pid) {
    int vma_count;
    while (getline(inFile, curLine)) {
      if (curLine[0] == '#') continue;

      istringstream streamLine(curLine);
      streamLine >> vma_count;
      break;
    }
    Process* process = new Process(pid, PT_SIZE, vma_count);
    process_table[pid] = process;

    for (int i = 0; i < vma_count; ++i) {
      getline(inFile, curLine);
      istringstream streamLine(curLine);
      VMA vma;
      streamLine >> vma.start_vpage >> vma.end_vpage >> vma.write_protected >> vma.file_mapped;
      process->vmas[i] = vma;
    }
  }

  while (getline(inFile, curLine)) {
    if (curLine[0] == '#') continue;

    istringstream streamLine(curLine);
    Instruction inst;
    streamLine >> inst.instruction >> inst.vpage;
    instructions.push_back(inst);
  }

  inFile.close();
}

Frame* getNextFrame() {
  if (!free_pool.empty()) {
    Frame* frame = free_pool.front();
    free_pool.pop();
    return frame;
  }
  if (isRandom) {
    return frame_table[myrandom(num_frames)];
  }

  return pager->select_victim_frame();
}

void processInstructions() {
  inst_count = instructions.size();
  for (int i = 0; i < instructions.size(); ++i) {
    Instruction* instr = &instructions[i];
    if (optionO) {
      printf("%d: ==> %c %d \n", i, instr->instruction, instr->vpage);
    }
    switch(instr->instruction) {
      case 'c':
      { 
        current_proc = process_table[instr->vpage];
        ctx_switches++;
        break;
      }
      case 'e':
      { 
        releaseFrames();
        current_proc = nullptr;
        process_exits++;
        break;
      }
      default:
      { 
        PTE* pte = &current_proc->page_table[instr->vpage];
        if (!pte->PRESENT) {
          // generates the page fault exception
          // verify this is actually a valid page in a vma if not raise error and next inst
          if (pte->VMA_CHECKED == 0) {
            // 0 - not checked
            for (int j = 0; j < current_proc->vma_num; ++j) {
              VMA vma = current_proc->vmas[j];
              if (instr->vpage >= vma.start_vpage && instr->vpage <= vma.end_vpage) {
                pte->WRITE_PROTECT = vma.write_protected;
                pte->FILE_MAPPED = vma.file_mapped;
                // 1 - checked and in vmas
                pte->VMA_CHECKED = 1;
                break;
              }
            }
            if (pte->VMA_CHECKED == 0) {
              // 2 - checked and NOT in vmas
              pte->VMA_CHECKED = 2;
            }
          }
          if (pte->VMA_CHECKED == 2) {
            if (optionO) {
              cout << " SEGV" << endl;
            }
            current_proc->segv++;
            break;
          }
          
          // 1 - checked and in vmas
          if (!isRandom) {
            pager->instr = i;
          }

          Frame* newframe = getNextFrame();
          // figure out if/what to do with old frame if it was mapped
          // UNMAP, OUT/FOUT
          if (newframe->pid != -1) {
            if (optionO) {
              printf(" UNMAP %d:%d \n", newframe->pid, newframe->vpage);
            }
            process_table[newframe->pid]->unmaps++;
            process_table[newframe->pid]->page_table[newframe->vpage].PRESENT = 0;

            PTE* old_pte = &(process_table[newframe->pid]->page_table)[newframe->vpage];
            if (old_pte->MODIFIED) {
              old_pte->MODIFIED = 0;
              if (old_pte->FILE_MAPPED) {
                if (optionO) {
                  cout << " FOUT" << endl;
                }
                process_table[newframe->pid]->fouts++;
              } else {
                if (optionO) {
                  cout << " OUT" << endl;
                }
                process_table[newframe->pid]->outs++;
              }
              old_pte->PAGEDOUT = 1;
            }
          }
          
          // see whether and how to bring in the content of the access page.
          pte->PRESENT = 1;
          pte->FRAME = newframe->fid;
          newframe->pid = current_proc->pid;
          newframe->vpage = instr->vpage;
          newframe->pte = pte;
          if (!isRandom) pager->addFrame(newframe);
          
          // ZERO/IN/FIN
          if (pte->FILE_MAPPED) {
            if (optionO) {
              cout << " FIN" << endl;
            }
            current_proc->fins++;
          } else if (pte->PAGEDOUT) {
            if (optionO) {
              cout << " IN" << endl;
            }
            current_proc->ins++;
          } else {
            if (optionO) {
              cout << " ZERO" << endl;
            }
            current_proc->zeros++;
          }

          // MAP
          if (optionO) {
            printf(" MAP %d \n", newframe->fid);
          }
          current_proc->maps++;
          newframe->age = 0;
        }

        // update_pte(read/modify) bits based on operations.
        pte->REFERENCED = 1;
        frame_table[pte->FRAME]->lastRef = i;
        if (instr->instruction == 'w') {
          if (pte->WRITE_PROTECT) {
            // check write protection
            if (optionO) {
              cout << " SEGPROT" << endl;
            }
            current_proc->segprot++;
          } else {
            pte->MODIFIED = 1;
          }
        } else {
          // pte->MODIFIED = 0;
        }
        break;
      }
    }
  }
}

void releaseFrames() {
  if (optionO) {
    printf("EXIT current process %d \n", current_proc->pid);
  }
  for (int i = 0; i < PT_SIZE; ++i) {
    PTE* pte = &(current_proc->page_table)[i];
    pte->PAGEDOUT = 0;
    Frame* frame = frame_table[pte->FRAME];

    if (!pte->PRESENT) continue;

    pte->PRESENT = 0;
    if (optionO) {
      printf(" UNMAP %d:%d \n", current_proc->pid, i);
    }
    current_proc->unmaps++;

    if (pte->MODIFIED && pte->FILE_MAPPED) {
      if (optionO) {
        cout << " FOUT" << endl;
      }
      current_proc->fouts++;
    }

    pte->PRESENT = 0;
    frame->pid = -1;
    frame->vpage = -1;
    free_pool.push(frame);
  }
}

void printFinalSummary() {
  if (optionP) {
    for (int i = 0; i < process_count; ++i) {
      printf("PT[%d]: ", i);
      PTE* pt = process_table[i]->page_table;
      for (int j = 0; j < PT_SIZE; ++j) {
        PTE pte = pt[j];
        if (!pte.PRESENT) {
          if (pte.PAGEDOUT && !pte.FILE_MAPPED) {
            cout << "# ";
          } else {
            cout << "* ";
          }
        } else {
          printf("%d:%c%c%c ", j, (pte.REFERENCED) ? 'R' : '-', 
                  (pte.MODIFIED) ? 'M' : '-',
                  (pte.PAGEDOUT && !pte.FILE_MAPPED) ? 'S' : '-');
        }
      }
      cout << endl;
    }
  }
  if (optionF) {
    cout << "FT: ";
    for (int i = 0; i < num_frames; ++i) {
      Frame* frame = frame_table[i];
      if (frame->pid == -1) {
        cout << "* ";
      } else {
        printf("%d:%d ", frame->pid, frame->vpage);
      }
    }
    cout << endl;
  }
  if (optionS) {
    unsigned long long cost = 0;

    for (int i = 0; i < process_count; ++i) {
      printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
              process_table[i]->pid,
              process_table[i]->unmaps, process_table[i]->maps, process_table[i]->ins, 
              process_table[i]->outs, process_table[i]->fins, process_table[i]->fouts, 
              process_table[i]->zeros, process_table[i]->segv, process_table[i]->segprot);
      
      cost += process_table[i]->maps * 300;
      cost += process_table[i]->unmaps * 400;
      cost += process_table[i]->ins * 3100;
      cost += process_table[i]->outs * 2700;
      cost += process_table[i]->fins * 2800;
      cost += process_table[i]->fouts * 2400;
      cost += process_table[i]->zeros * 140;
      cost += process_table[i]->segv * 340;
      cost += process_table[i]->segprot * 420;
    }
    
    cost += (inst_count + 129 * ctx_switches + 1249 * process_exits);
    printf("TOTALCOST %lu %lu %lu %llu %lu\n",
            inst_count, ctx_switches, process_exits, cost, sizeof(PTE));
  }
}