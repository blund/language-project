#include "stdio.h"

#include "ast.h"

void indent(parser* p) {
  printf("%*s", p->indent, " ");
}

void print_if_block(parser* p, if_block ib);
void print_block(parser* p, block* b);

void print_unit(parser* p, unit u) {
  for (int i = u.start; i < u.end; i++) {
    printf("%c", p->code[i]);
  }
}
void print_type(parser* p, type t) {
  print_unit(p, t.name);
  if (t.ptr) printf("*");
}

void print_assign(parser* p, assign a);

void print_value(parser* p, value v) {
  if (v.type == string_type) {
    print_unit(p, v.data);
    return;
  }
  if (v.type == integer_type) {
    print_unit(p, v.data);
    return;
  }
}

void print_call(parser* p, call c) {
  print_unit(p, c.name);
  printf("()");
}

void print_if_block(parser* p, if_block ib) {
  printf("if () {\n");
  p->indent += 2;

  print_block(p, ib.body);
  p->indent -= 2;

  indent(p);
  printf("}\n");
}

void print_expr(parser* p, expr* e) {
  if (e->kind == expr_value_kind) print_value(p, e->value);
  if (e->kind == expr_call_kind)  print_call(p,  e->call);
}

void print_statement(parser* p, statement* s) {
  indent(p);
  if (s->kind == statement_assign_kind) {
    print_assign(p, s->assign);
    printf(";\n");
  }
  if (s->kind == statement_call_kind) {
    print_call(p,   s->call);
    printf(";\n");
  }

  if (s->kind == statement_if_kind) {
    print_if_block(p, s->if_block);
  }
}

void print_assign(parser* p, assign a) {
  print_type(p, a.type);
  printf(" ");
  print_unit(p, a.name);
  printf(" = ");
  print_expr(p, a.expr);
}
void print_block(parser* p, block* b) {
  block* iter = b;

  // @TODO unsure about this logic, but it works B)
  for(;;) {
    print_statement(p, iter->statement);
    if (!iter->next) return;
    iter = iter->next;
  }
}

void print_func_decl(parser* p, func_decl* f) {
  print_type(p,  f->ret);
  printf(" ");
  print_unit(p,  f->name);
  printf("() {\n");
  p->indent += 2;
  print_block(p, f->body);
  printf("}\n");
}

