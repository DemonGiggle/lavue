#include <memory>
#include <string>
#include <map>

#include "ast.hh"
#include "utils.hh"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"


static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;
static std::map<std::string, Value*> NamedValues;


Value *NumberExprAST::codegen() {
  return nullptr;
}

Value *VariableExprAST::codegen() {
  return nullptr;
}

Value *BinaryExprAST::codegen() {
  return nullptr;
}

Value *CallExprAST::codegen() {
  return nullptr;
}

Value *PrototypeAST::codegen() {
  return nullptr;
}

Value *FunctionAST::codegen() {
  return nullptr;
}
