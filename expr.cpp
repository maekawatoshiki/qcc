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
    AST *rhs = expr_asgmt();
    if(token.get().type == TOK_TYPE_SYMBOL) {
      next_prec = get_op_prec(token.get().val);
      if(tok_prec < next_prec) 
        rhs = expr_rhs(tok_prec + 1, rhs);
    } 
    lhs = new BinaryAST(op, lhs, rhs);    
  }
}

AST *Parser::expr_entry() {
  return expr_rhs(0, expr_asgmt());
}

AST *Parser::expr_asgmt() {
  AST *lhs, *rhs;
  lhs = expr_unary();
  std::string op = token.get().val;
  while(op == "+=" ||
      op == "-=" ||
      op == "*=" ||
      op == "/=" ||
      op == "&=" ||
      op == "|=" ||
      op == "^=" ||
      op == "=") {
    bool add = op == "+=", sub = op == "-=", mul = op == "*=", div = op == "/=", 
         aand = op == "&=", aor = op == "|=", axor = op == "^=", normal = op == "=";
    token.skip();
    rhs = expr_entry();
    lhs = new AsgmtAST(lhs, normal ? rhs :
        new BinaryAST(
          add ? "+" : 
          sub ? "-" : 
          mul ? "*" : 
          div ? "/" : 
          aand? "&" : 
          aor ? "|" :
          axor? "^" : "ERROR", lhs, rhs));  
    op = token.get().val;
  }
  return lhs;
}

AST *Parser::expr_unary() {
  bool op_addr = false, op_aste = false;
  op_addr = token.is("&");
  op_aste = token.is("*");
  if(op_addr||op_aste) token.skip();
  AST *expr;
  if(op_addr || op_aste) {
    expr = expr_unary();
    return new UnaryAST(op_addr ? "&" : 
                        op_aste ? "*" : "", expr);
  } else 
    expr = expr_dot();
  return expr;
}

AST *Parser::expr_dot() {
  return expr_index();
}

AST *Parser::expr_index() {
  AST *lhs, *rhs;
  lhs = expr_primary();
  while(token.skip("[")) {
    rhs = expr_entry();
    token.skip("]");
    lhs = new IndexAST(lhs, rhs);
  }
  return lhs;
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
  } else if(token.skip("(")) {
    auto e = expr_entry();
    token.skip(")");
    return e;
  }
  return nullptr;
}
