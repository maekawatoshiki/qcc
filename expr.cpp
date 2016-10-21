#include "expr.hpp"

int Parser::get_op_prec(std::string op) {
  return op_prec.count(op) != 0 ? op_prec[op] : -1;
}

AST *Parser::expr_rhs(int prec, AST *lhs) {
  while(true) {
    int tok_prec, next_prec;
    if(token.get().type == TOK_TYPE_SYMBOL) {
      tok_prec = get_op_prec(token.get().val);
      if(tok_prec < prec) return lhs;
    } else return lhs;
    std::string op = token.next().val;
    AST *rhs = expr_entry();
    if(token.get().type == TOK_TYPE_SYMBOL) {
      next_prec = get_op_prec(token.get().val);
      if(tok_prec < next_prec) 
        rhs = expr_rhs(tok_prec + 1, rhs);
    } 
    if(op == "+=" ||
        op == "-=" ||
        op == "*=" ||
        op == "/=" ||
        op == "&=" ||
        op == "|=" ||
        op == "^=" ||
        op == "=") {
      bool add = op == "+=", sub = op == "-=", mul = op == "*=", div = op == "/=", 
           aand = op == "&=", aor = op == "|=", axor = op == "^=", normal = op == "=";
      lhs = new AsgmtAST(lhs, normal ? rhs :
            new BinaryAST(
              add ? "+" : 
              sub ? "-" : 
              mul ? "*" : 
              div ? "/" : 
              aand? "&" : 
              aor ? "|" :
              axor? "^" : "ERROR", lhs, rhs));  
    } else
      lhs = new BinaryAST(op, lhs, rhs);    
  }
  return lhs;
}

AST *Parser::expr_entry() {
  return expr_rhs(0, expr_dot());
}

AST *Parser::expr_dot() {
  return expr_index();
}

AST *Parser::expr_index() {
  return expr_unary();
}

AST *Parser::expr_unary() {
  return expr_primary();
}

AST *Parser::expr_primary() {
  if(token.get().type == TOK_TYPE_NUMBER) {
    return new NumberAST(atoi(token.next().val.c_str()));
  } else if(token.get().type == TOK_TYPE_STRING) {
    return new StringAST(token.next().val);
  } else if(token.get().type == TOK_TYPE_IDENT) {
    std::string name = token.next().val;
    if(token.skip("(")) { // function?
      AST_vec args;
      while(!token.skip(")")) {
        args.push_back(expr_entry());
        token.skip(",");
      }
      // token.skip(";");
      return new FunctionCallAST(name, args);
    } else { // variable
      return new VariableAST(name);
    }
  }
  return nullptr;
}
