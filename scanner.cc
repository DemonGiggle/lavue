#include <iostream>
#include <string>

enum Token {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,
};

static std::string IdentifierStr;    // Filled in if tok_identifier
static double NumVal;                // Filled in if tok_number

static bool isalphanum(int c) {
  if (c >= '0' && c <= '9') 
    return true;

  if (isalpha(c)) return true;

  return false;
}

// gettok - Return the next token from standard input
static int gettok() {
  static int LastChar = ' ';

  // strip white spaces
  while (isspace(LastChar)) {
    LastChar = getchar();
  }

  // something starts with an alphabet
  if (isalpha(LastChar)) {
    IdentifierStr = LastChar;

    // read a whole token until non-num or alphabets
    while (isalphanum(LastChar = getchar())) {
      IdentifierStr += LastChar;
    }

    if (IdentifierStr == "def") {
      return tok_def;
    }

    if (IdentifierStr == "extern") {
      return tok_extern;
    }

    return tok_identifier;
  }

  // something start with num related chars
  if (isdigit(LastChar) || LastChar == '.') {
    std::string numStr;
    do {
      numStr += LastChar;
      LastChar = getchar();
    } while (isdigit(LastChar) || LastChar == '.');
    NumVal = strtod(numStr.c_str(), 0);
    return tok_number;
  }

  // handle comment
  if (LastChar == '#') {
    do {
      LastChar = getchar();
    } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF) return gettok();
  }

  // remaining
  if (LastChar == EOF) {
    return tok_eof;
  }

  int c = LastChar;
  LastChar = getchar();
  return c;
}

int main() {
  int tok;

  while ((tok = gettok()) != tok_eof) {
    switch(tok) {
      case tok_def: std::cout << "tok_def" << std::endl; break;
      case tok_extern: std::cout << "tok_extern" << std::endl; break;
      case tok_identifier: std::cout << "tok_identifier" << std::endl; break;
      case tok_number: std::cout << "tok_number" << std::endl; break;
    }
  }
}
