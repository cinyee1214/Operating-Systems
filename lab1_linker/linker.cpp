#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <map>
#include <set>
#include <iterator> 
#include <algorithm>
#include <cmath>
#include <climits>

using namespace std;

struct Token {
  int row, col;
  string token; 
};

struct Symbol {
  int abs_addr;
  string error = "";
  int module;
};

void passOne(char* file);
void passTwo(char* file);
void parseError(int errCode);
void getToken();
int readInt();
string readSym();
char readIEAR();
void printSymbolTable();
void printWarnings();
void printLastWarnings();
bool isSymbol(string token);
void processInst(int addr, char addressmode, int instruction, int module_size);
bool sortByNum(const pair<string, int> &a, const pair<string, int> &b);

map<string, Symbol> symbolTable;
map<string, Symbol>::iterator itr; 
vector<string> curUseList;
vector<bool> whetherUsed;
vector<string> warningList;
set<string> usedList;  
Token curToken;
int tokenIndex;
ifstream inFile;
string curLine;
int row = 0; 
int col = 0;
char *str;
int num_instr;
int module;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    cout << "Please pass an argument." << endl;
    exit(EXIT_FAILURE);
  }

  passOne(argv[1]);
  cout << endl;
  
  cout << "Memory Map" << endl;
  passTwo(argv[1]);
  printLastWarnings();

  return 0;
}

void passOne(char* file) {
  inFile.open(file);
  num_instr = 0;
  module = 1;

  while (!inFile.eof()) {
    // definition list
    int defCount = readInt();
    if (defCount == -1) break;
    if (defCount > 16) {
      parseError(4);
    }
    
    vector<string> definedSyms;
    for (int i = 0; i < defCount; ++i) {
      string sym = readSym();
      int val = readInt();
      itr = symbolTable.find(sym);
      if (itr != symbolTable.end()) {
        itr->second.error = "Error: This variable is multiple times defined; first value used";
      } else {
        Symbol symbol;
        symbol.abs_addr = num_instr + val;
        symbol.module = module;
        symbolTable.insert(pair<string, Symbol>(sym, symbol));
        definedSyms.push_back(sym);
      }
    }

    // use list
    int useCount = readInt();
    if (useCount > 16) {
      parseError(5);
    }
    for (int i = 0; i < useCount; ++i) {
      string sym = readSym();
    }

    // program list
    int instCount = readInt();
    if (instCount == -1) parseError(0);
    if (num_instr + instCount > 512) {
      parseError(6);
    }
    for (int i = 0; i < instCount; ++i) {
      char addressmode = readIEAR();
      int instruction = readInt();
    }

    // warning:
    for (int i = 0; i < definedSyms.size(); ++i) {
      itr = symbolTable.find(definedSyms[i]);
      int idx = itr->second.abs_addr;
      if (idx - num_instr >= instCount) {
        printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n", module, definedSyms[i].c_str(), idx - num_instr, instCount - 1);
        itr->second.abs_addr = num_instr;
      }
    }

    num_instr += instCount;
    module++;
  }
  printSymbolTable();
  inFile.close();
}

void passTwo(char* file) {
  inFile.open(file);
  num_instr = 0;
  module = 1;

  while (!inFile.eof()) {
    // definition list
    int defCount = readInt();
    if (defCount == -1) break;

    for (int i = 0; i < defCount; ++i) {
      string sym = readSym();
      int val = readInt();
    }

    // use list
    int useCount = readInt();
    curUseList.clear();
    whetherUsed.clear();
    for (int i = 0; i < useCount; ++i) {
      string sym = readSym();
      curUseList.push_back(sym);
      whetherUsed.push_back(false);
    }

    // program list
    int instCount = readInt();
    for (int i = 0; i < instCount; ++i) {
      char addressmode = readIEAR();
      int instruction = readInt();
      processInst(i + num_instr, addressmode, instruction, instCount);
    }
    num_instr += instCount;
    printWarnings();
    module++;
  }

  inFile.close();
}

void processInst(int addr, char addressmode, int instruction, int module_size) {
  string error_msg = "";
  if (instruction >= 10000) {
    instruction = 9999;
    if (addressmode == 'I') {
      error_msg = "Error: Illegal immediate value; treated as 9999";
    } else {
      error_msg = "Error: Illegal opcode; treated as 9999";
    }
    cout << setfill('0') << setw(3) << addr << ": " << setfill('0') << setw(3) << instruction << " " << error_msg << endl;
    return;
  }

  int opcode = instruction / 1000;
  int operand = instruction % 1000;
  switch (addressmode) {
    case 'A':
    {      
      if (operand >= 512) {
        error_msg = "Error: Absolute address exceeds machine size; zero used";
        instruction = opcode * 1000;
      }
      break;
    }
    case 'R':
    {
      if (operand >= module_size) {
        error_msg = "Error: Relative address exceeds module size; zero used";
        instruction = opcode * 1000 + num_instr;
      } else {
        instruction += num_instr;
      }
      break;
    }
    case 'E':
    {
      if (operand >= curUseList.size()) {
        error_msg = "Error: External address exceeds length of uselist; treated as immediate";
        break;
      }

      whetherUsed[operand] = true;

      string cur = curUseList[operand];
      itr = symbolTable.find(cur);
      if (itr == symbolTable.end()) {
        cout << setfill('0') << setw(3) << addr << ": " << setfill('0') << setw(4) << instruction;
        printf(" Error: %s is not defined; zero used\n", cur.c_str());
        return;
      } else {
        usedList.insert(cur);
        instruction = opcode * 1000 + itr->second.abs_addr;
      }
      break;
    }
    default:  // 'I'
      break;
  }
  cout << setfill('0') << setw(3) << addr << ": " << setfill('0') << setw(4) << instruction << " " << error_msg << endl;
}

int readInt() {
  getToken();
  if (curToken.token == "-1") return -1;
  if (curToken.token == "" || !isdigit(curToken.token[0]) || stol(curToken.token) > INT_MAX) {
    parseError(0);
  }
  return stoi(curToken.token);
}

string readSym() {
  getToken();
  if (curToken.token == "" || curToken.token == "-1" || !isSymbol(curToken.token)) {
    parseError(1);
  }
  if (curToken.token.length() > 16) {
    parseError(3);
  }
  return curToken.token;
}

bool isSymbol(string token) {
  if (!isalpha(token[0])) return false;
  for (int i = 1; i < token.length(); ++i) {
    if (!isalnum(token[i])) return false;
  }
  return true;
}

char readIEAR() {
  getToken();
  if (curToken.token == "" || curToken.token.length() != 1 || curToken.token == "-1" || 
  (curToken.token[0] != 'I' && curToken.token[0] != 'E' && curToken.token[0] != 'A' && curToken.token[0] != 'R')) {
    parseError(2);
  }
  return curToken.token[0];
}

void getToken() {
  while (str == NULL && getline(inFile, curLine)) {
    row++;
    col = 0;
    if (curLine == "" || curLine == "\n" || curLine == "\t") {
      curToken.token = "";
      curToken.row = row;
      curToken.col = col + 1;
    } else {
      str = strtok(&curLine[0], " \t\n");
    }
  } 

  if (inFile.eof()) {
    curToken.token = "-1";
    curToken.row = row;
    curToken.col = col + 1;
    return;
  }

  // Now, str != NULL
  curToken.row = row;
  curToken.col = curLine.find(string(str), col) + 1;
  curToken.token = str;
  col = curToken.col + strlen(str) - 1;
  str = strtok(NULL, " \t\n");
}

void parseError(int errCode) {
  static char* errStr[] = {
    (char *)"NUM_EXPECTED",   // Number expect
    (char *)"SYM_EXPECTED",   // Symbol Expected
    (char *)"ADDR_EXPECTED",  // Addressing Expected which is A/E/I/R 
    (char *)"SYM_TOO_LONG",   // Symbol Name is too long
    (char *)"TOO_MANY_DEF_IN_MODULE",  //>16
    (char *)"TOO_MANY_USE_IN_MODULE",  //>16
    (char *)"TOO_MANY_INSTR" // total num_instr exceeds memory size (512)
  };
  printf("Parse Error line %d offset %d: %s\n", curToken.row, curToken.col, errStr[errCode]); 
  exit(EXIT_FAILURE);
}

void printSymbolTable() {
  cout << "Symbol Table" << endl;
  vector< pair<string, int> > vec;
  for (itr = symbolTable.begin(); itr != symbolTable.end(); ++itr) {
    vec.push_back(make_pair(itr->first, itr->second.abs_addr));
  }
  sort(vec.begin(), vec.end(), sortByNum);
  for (int i = 0; i < vec.size(); ++i) {
		cout << vec[i].first << "=" << vec[i].second << " " << symbolTable.find(vec[i].first)->second.error << endl;
	}
}

bool sortByNum(const pair<string, int> &a, const pair<string, int> &b) { 
  return (a.second < b.second); 
} 

void printWarnings() {
  for (int i = 0; i < whetherUsed.size(); ++i) {
    if (!whetherUsed[i]) {
      printf("Warning: Module %d: %s appeared in the uselist but was not actually used\n", 
      module, curUseList[i].c_str()); 
    }
  }
}

void printLastWarnings() {
  cout << endl;
  vector< pair<string, int> > vec;
  for (itr = symbolTable.begin(); itr != symbolTable.end(); ++itr) {
    vec.push_back(make_pair(itr->first, itr->second.module));
  }
  sort(vec.begin(), vec.end(), sortByNum);
  for (int i = 0; i < vec.size(); ++i) {
    if (usedList.find(vec[i].first) == usedList.end()) {
      printf("Warning: Module %d: %s was defined but never used\n", symbolTable.find(vec[i].first)->second.module, symbolTable.find(vec[i].first)->first.c_str()); 
    }
	}
  cout << endl;
}