// base class of all AST node
class ExprAST {
  public:
    virtual ~ExprAST() {}
}

class NumberExprAST : public ExprAST {
  double Val;

  public:
    NumberExprAST(double val) : Val(val) {}
}

class VariableExprAST : public ExprAST {
  std::string Name;

  public:
    VariableExprAST(std::string &name) : Name(name) {}
}

class BinaryExprAST : public ExprAST {
  char Op;
  std::unique_ptr<ExprAST> LHS, RHS;

  public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
      : Op(op), LHS(std::move(lhs)), RHS(std::move(rhs)) {}
}

class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

  public:
    CallExprAST(const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args) 
      : Callee(callee), Args(std::move(args)) {}
}

