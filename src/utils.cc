#include "utils.hh"

std::unique_ptr<ExprAST> LogError(const char * str) {
  fprintf(stderr, "LogError: %s\n", str);
  return nullptr;
}

Value *LogErrorV(const char *str) {
  LogError(str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char * str) {
  LogError(str);
  return nullptr;
}
