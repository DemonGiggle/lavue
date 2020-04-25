#include <memory>
#include <string>
#include <map>

#include "ast.hh"
#include "utils.hh"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"


static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;
static std::map<std::string, Value*> NamedValues;

void LLVMModuleSetup() {
  TheModule = std::make_unique<Module>("lavue jit", TheContext);
}

void LLVMModuleDump() {
  TheModule->print(errs(), nullptr);
}


Value *NumberExprAST::codegen() {
  return ConstantFP::get(TheContext, APFloat(Val));
}

Value *VariableExprAST::codegen() {
  // look this variable up to the function
  // in current design, variables are only declared in function parameters
  Value *v = NamedValues[Name];
  if (!v) 
    LogErrorV("Unknown variable name");
  return v;
}

Value *BinaryExprAST::codegen() {
  Value *l = LHS->codegen();
  Value *r = RHS->codegen();
  if (!l || !r)
    return nullptr;

  switch(Op) {
  case '+':
    return Builder.CreateFAdd(l, r, "addtmp");
  case '-':
    return Builder.CreateFSub(l, r, "subtmp");
  case '*':
    return Builder.CreateFMul(l, r, "multmp");
  case '<':
    l = Builder.CreateFCmpULT(l, r, "cmptmp");
    // convert bool 0/1 to double 
    return Builder.CreateUIToFP(l, Type::getDoubleTy(TheContext), "booltmp");
  default:
    return LogErrorV("invalid binary operator");
  }
}

Value *CallExprAST::codegen() {
  // lookup the name in global module table
  Function *calleef = TheModule->getFunction(Callee);
  if (!calleef) {
    return LogErrorV("unknown function referenced");
  }

  // check argument counts
  if (calleef->arg_size() != Args.size()) {
    return LogErrorV("incorrect number of arguments");
  }

  std::vector<Value*> argsv;
  for (auto& arg: Args) {
    argsv.push_back( arg->codegen() );
    if (!argsv.back())
      return nullptr;
  }

  return Builder.CreateCall(calleef, argsv, "calltmp");
}

Function *PrototypeAST::codegen() {
  // in current design, all function arguments are double
  std::vector<Type*> argTypes(Args.size(), Type::getDoubleTy(TheContext));

  // in current design, the return type is double
  FunctionType* ft = 
    FunctionType::get(Type::getDoubleTy(TheContext), argTypes, false);

  Function* f = 
    Function::Create(ft, Function::ExternalLinkage, Name, TheModule.get());

  // set name of each argument
  unsigned idx = 0;
  for (auto& arg: f->args()) {
    arg.setName(Args[idx++]);
  }

  return f;
}

Value *FunctionAST::codegen() {
  Function* function = TheModule->getFunction(Proto->getName());

  // this style of writing codes, means that we cannot make sure when
  // the prototype is codegen. Unless the prototype can be written somewhere
  // without body definition. (like c/c++ header)
  if (!function) {
    function = Proto->codegen();
  }

  if (!function) {
    return nullptr;
  }

  if (!function->empty()) {
    return (Function*)LogErrorV("function cannot be redefined");
  }

  BasicBlock *bb = BasicBlock::Create(TheContext, "entry", function);
  Builder.SetInsertPoint(bb);

  // record the function argument names
  NamedValues.clear();
  for (auto& arg: function->args()) {
    NamedValues[arg.getName()] = &arg;
  }

  if (Value* ret = Body->codegen()) {
    // finish off the function
    Builder.CreateRet(ret);

    // validate the generated code, checking for consistency
    verifyFunction(*function);

    return function;
  }

  // error reading body, remove function
 // otherwise, the invalid function would still in the module symbol table
  function->eraseFromParent();
  return nullptr;
}
