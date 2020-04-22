#include <memory>
#include <vector>
#include "llvm/IR/Value.h"

using namespace llvm;

// base class of all AST node
class ExprAST {
  public:
    virtual ~ExprAST() = default;
    virtual Value *codegen() = 0;
};

class NumberExprAST : public ExprAST {
  double Val;

  public:
    NumberExprAST(double val) : Val(val) {}
    virtual Value *codegen();
};

class VariableExprAST : public ExprAST {
  std::string Name;

  public:
    VariableExprAST(std::string &name) : Name(name) {}
    virtual Value *codegen();
};

class BinaryExprAST : public ExprAST {
  char Op;
  std::unique_ptr<ExprAST> LHS, RHS;

  public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
      : Op(op), LHS(std::move(lhs)), RHS(std::move(rhs)) {}
    virtual Value *codegen();
};

class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

  public:
    CallExprAST(const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args) 
      : Callee(callee), Args(std::move(args)) {}
    virtual Value *codegen();
};

// This is function declaration part, which captures the name of the function and its
// arguments
class PrototypeAST : public ExprAST {
  std::string Name;
  std::vector<std::string> Args;

  public:
    PrototypeAST(const std::string& name, std::vector<std::string> args)
      : Name(name), Args(std::move(args)) {}

    const std::string &getName() const { return Name; }
    virtual Value *codegen();
};

// Function definition
class FunctionAST : public ExprAST {
  std::unique_ptr<PrototypeAST> Proto;
  std::unique_ptr<ExprAST> Body;

  public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
      : Proto(std::move(proto)), Body(std::move(body)) {}
    virtual Value *codegen();
};
