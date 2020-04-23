#ifndef UTILS_H
#define UTILS_H

#include <memory>
#include "ast.hh"
#include "llvm/IR/Value.h"

using namespace llvm;

Value *LogErrorV(const char *str);
std::unique_ptr<ExprAST> LogError(const char * str);
std::unique_ptr<PrototypeAST> LogErrorP(const char * str);

#endif
