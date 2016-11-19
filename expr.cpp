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
  bool op_addr = false, op_aste = false, op_inc = false, op_dec = false;
  op_addr = token.is("&");
  op_aste = token.is("*");
  op_inc = token.is("++");
  op_dec = token.is("--");
  AST *expr = nullptr;
  if(op_addr || op_aste || op_inc || op_dec) {
    token.skip();
    expr = expr_unary();
    return new UnaryAST(op_addr ? "&" : 
                        op_aste ? "*" :
                        op_inc  ? "++":
                        op_dec  ? "--" : "", expr);
  } else 
    expr = expr_dot();
  return expr_unary_postfix(expr);
}

AST *Parser::expr_unary_postfix(AST *expr) {
  bool op_inc = token.is("++"), op_dec = token.is("--");
  if(op_inc || op_dec) {
    token.skip();
    return new UnaryAST(op_inc ? "++" : 
                        op_dec ? "--" : "", expr, /*postfix=*/true);
  }
  return expr;
}

AST *Parser::expr_dot() {
  AST *lhs, *rhs;
  bool arrow = false;
  lhs = expr_index();
  while((arrow=token.skip("->")) || token.skip(".")) {
    rhs = expr_primary();
    lhs = new DotOpAST(lhs, rhs, arrow);
    while(token.skip("[")) {
      rhs = expr_entry();
      token.skip("]");
      lhs = new IndexAST(lhs, rhs);
    }
    arrow = false;
  }
  return lhs;
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
  } else if(token.get().type == TOK_TYPE_CHAR) {
    return new NumberAST(token.next().val[0]);
  } else if(token.skip("sizeof")) {
    token.expect_skip("(");
    llvm::Type *type; 
    if(is_type()) {
      type = skip_type_spec(); // base type
      std::string _; 
      type = read_declarator(_, type);
    }
    token.expect_skip(")");
    return new SizeofAST(type);
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
  } else if(token.skip("{")) {
    AST_vec elems;
    while(token.get().type != TOK_TYPE_END) {
      elems.push_back(expr_entry());
      if(token.skip("}")) break;
      token.expect_skip(",");
    }
    return new ArrayAST(elems);
  } else if(token.skip("(")) {
    auto e = expr_entry();
    token.skip(")");
    return e;
  }
  return nullptr;
}
