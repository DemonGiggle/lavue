#include <iostream>
#include <string>
#include <memory>
#include <map>
#include "ast.hh"

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
static int CurTok;                   // Current token the parser looks at
static std::map<char, int> BinopPrecedence;  // Binary operation precedence

static std::unique_ptr<ExprAST> ParseExpression();

static int GetTokPrecedence() {
  if (!isascii(CurTok)) 
    return -1;

  int tokPrec = BinopPrecedence[CurTok];
  if (tokPrec <= 0) return -1;
  return tokPrec;
}

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
    NumVal = strtod(numStr.c_str(), nullptr);
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

std::unique_ptr<ExprAST> LogError(const char * str) {
  fprintf(stderr, "LogError: %s\n", str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char * str) {
  LogError(str);
  return nullptr;
}

static int getNextToken() {
  return CurTok = gettok();
}

/// numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr() {
  auto result = std::make_unique<NumberExprAST>(NumVal); 
  getNextToken();
  return std::move(result);
}

/// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken(); // eat '('
  auto v = ParseExpression();

  if (!v) {
    return nullptr;
  }

  if (CurTok != ')') {
    return LogError("expect ')'");
  }

  getNextToken(); // eat ')'
  return v;
}

/// identifierexpr 
///   ::= identifier
///   ::= identifier '(' expression* ')'
///
/// ex: hello = x
///     hello = y(x, 1)
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
  std::string idName = IdentifierStr;
  getNextToken();     // eat identifier

  // simple identifier
  if (CurTok != '(') {
    return std::make_unique<VariableExprAST>(idName);
  }

  // function call
  getNextToken();    // eat '('
  std::vector<std::unique_ptr<ExprAST>> args;

  // parse arguments if something in paren
  if (CurTok != ')') {
    while (1) {
      if (auto arg = ParseExpression()) 
        args.push_back(std::move(arg));
      else
        return nullptr;

      if (CurTok == ')') {
        break;
      }

      if (CurTok != ',') {
        return LogError("expect ')' or ',' in argument list");
      }

      getNextToken();
    }
  }
  getNextToken(); // eat ')'

  return std::make_unique<CallExprAST>(idName, std::move(args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary() {
  switch (CurTok) {
    default:
      return LogError("unknown token when expecting an expression");
    case tok_identifier:
      return ParseIdentifierExpr();
    case tok_number:
      return ParseNumberExpr();
    case '(':
      return ParseParenExpr();
  }
}


/// binoprhs
///   ::= ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(
    int exprPrec, 
    std::unique_ptr<ExprAST> lhs) {

  // find its precedence
  while (1) {
    int tokPrec = GetTokPrecedence();

    if (tokPrec  < exprPrec) {
      return lhs;
    }

    // okay, we know this is a binop
    int binop = CurTok;
    getNextToken(); // eat binop

    // parse the primary expression after the binary operator
    auto rhs = ParsePrimary();
    if (!rhs) return nullptr;

    int nextPrec = GetTokPrecedence();
    if (tokPrec < nextPrec) {
      rhs = ParseBinOpRHS(tokPrec + 1, std::move(rhs));
      if (!rhs) return nullptr;
    }

    lhs = std::make_unique<BinaryExprAST>(binop, std::move(lhs), std::move(rhs));
  }
}

/// expression
///   ::= primary binoprhs
static std::unique_ptr<ExprAST> ParseExpression() {
  auto lhs = ParsePrimary();
  if (!lhs)
    return nullptr;

  return ParseBinOpRHS(0, std::move(lhs));
}


/// prototype
///   ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {
  if (CurTok != tok_identifier) 
    return LogErrorP("expect function name in prototype");

  std::string fnName = IdentifierStr;
  getNextToken();

  if (CurTok != '(') 
    return LogErrorP("expect '(' in prototype");

  // read the list of argument names
  std::vector<std::string> argNames;
  while (getNextToken() == tok_identifier) {
    argNames.push_back(IdentifierStr);
  }

  if (CurTok != ')')
    return LogErrorP("expect ')' in prototype");

  // success
  getNextToken(); // eat ')'

  return std::make_unique<PrototypeAST>(fnName, std::move(argNames));
}

/// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition() {
  getNextToken(); // eat 'def'
  auto proto = ParsePrototype();
  if (!proto) return nullptr;

  if (auto e = ParseExpression()) {
    return std::make_unique<FunctionAST>(std::move(proto), std::move(e));
  }

  return nullptr;
}

/// extern ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern() {
  getNextToken(); // eat 'extern'
  return ParsePrototype();
}

/// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
  if (auto e = ParseExpression()) {
    auto proto = std::make_unique<PrototypeAST>("__anon_expr", 
                                                std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(proto), std::move(e));
  }
  return nullptr;
}

static void HandleDefinition() {
  if (ParseDefinition()) {
    fprintf(stderr, "Parse a function definition\n");
  } else {
    // error recovery
    getNextToken();
  }
}

static void HandleExtern() {
  if (ParseExtern()) {
    fprintf(stderr, "Parse an extern\n");
  } else {
    // error recovery
    getNextToken();
  }
}

static void HandleTopLevelExpression() {
  if (ParseTopLevelExpr()) {
    fprintf(stderr, "Parse top-level expr\n");
  } else {
    // error recovery
    getNextToken();
  }
}

/// top ::= definition | external | expression | ';'
static void MainLoop() {
  while (1) {
    fprintf(stderr, "ready> ");
    switch (CurTok) {
      case tok_eof: 
        return;
      case ';':
        getNextToken();
        break;
      case tok_def: 
        HandleDefinition();
        break;
      case tok_extern:
        HandleExtern();
        break;
      default:
        HandleTopLevelExpression();
        break;
    }
  }
}

int main() {
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;  // highest precedence

  fprintf(stderr, "ready> ");
  getNextToken();

  MainLoop();

  return 0;
}
