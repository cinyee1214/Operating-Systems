#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <map>
#include <algorithm>

using namespace std;

struct Token {
  int row, col;
  string token; 
};

void getToken(char* file);

vector<Token> tokens;
ifstream inFile;

int main (int argc, char* argv[]) {
  if (argc != 2) {
    cout << "Unexpected input format." << endl;
    exit(0);
  }

  getToken(argv[1]);
  cout << "The tokens are:" << endl;

  for (Token token : tokens) {
    cout << "Token: " << token.row << ":" << token.col << " : " << token.token << endl;
  }

  return 0;
}

void getToken(char* file) {
  inFile.open(file);
  string curLine;
  int row = 1;
  while (getline(inFile, curLine)) {
    char *str = strtok(&curLine[0], " \t\n");
    int col = 0;
    while (str != NULL) {
      Token curToken;
      curToken.row = row;
      curToken.col = curLine.find(string(str), col) + 1;
      curToken.token = str;
      tokens.push_back(curToken);
      col = curToken.col + strlen(str) - 1;
      str = strtok(NULL, " \t\n");
    }
    row++;
  }
  inFile.close();
}

