#include "string.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>

#include "ast.h"
#include "parse.h"

void parse_whitespace(parser* p) {
  if (p->index >= p->len) return;
  while (strchr(" \\\n\t", THIS)) p->index++;
}

unit parse_text(parser* p) {
  parse_whitespace(p);
  p->ok = 1;
  unit u = {.start = p->index};

  while ((THIS >= 'A' && THIS < 'Z') || (THIS >= 'a' && THIS <= 'z')) {
    p->index++;
  }

  u.end = p->index;
  return u;
}

value parse_number(parser* p) {
  parse_whitespace(p);
  p->ok = 1; // Reset index
  int i = p->index; // Save index

  value v;
  v.type = integer_type;
  
  unit u = {.start = p->index};

  // Return early if not ok
  if (THIS < '0' || THIS > '9') {
    p->ok = 0;
    return v;
  }
  
  while (THIS >= '0' && THIS <= '9'){
    p->index++;
  }

  u.end = p->index;
  v.data = u;
  return v;
}

void parse_exact(parser* p, char c) {
  parse_whitespace(p);

  p->ok = THIS == c;
  if (p->ok) p->index++;

  return;
}

value parse_string(parser* p) {
  parse_whitespace(p);
  p->ok = 1; // Reset index
  int i = p->index; // Save index

  value v;
  v.type = string_type;
  
  unit u = {.start = p->index};

  parse_exact(p, '"');
  while (THIS != '"') {
    p->index++;
  }
  parse_exact(p, '"');

  u.end = p->index;
  v.data = u;
  return v;
}

/* Types have a word and maybe a '*' */
type parse_type(parser* p) {
  parse_whitespace(p);
  type t;

  t.name = parse_text(p);

  // @TODO - handle pointer in type;
  parse_exact(p, '*');
  t.ptr = p->ok;

  return t;
}


call parse_call(parser* p) {
  parse_whitespace(p);
  p->ok = 1;
  int i = p->index;

  call c;

  c.name = parse_text(p);
  parse_exact(p, '(');
  parse_exact(p, ')');
  if (!p->ok) {
    p->index = i;
    return c;
  }
  
  return c;
}

expr* parse_expr(parser* p) {
  parse_whitespace(p);
  p->ok = 1;
  expr* e = malloc(sizeof(expr));

  e->kind = expr_call_kind;
  e->call = parse_call(p);

  if (!p->ok) {
    e->kind = expr_value_kind;
    e->value = parse_number(p);
  }

  if (!p->ok) {
    e->kind = expr_value_kind;
    e->value = parse_string(p);
  }
  return e;
}

assign parse_assign(parser* p) {
  parse_whitespace(p);
  p->ok = 1;
  int i = p->index;

  assign a;

  a.type = parse_type(p);
  a.name = parse_text(p);

  parse_exact(p, '=');
  if (!p->ok) {
    p->index = i;
    return a;
  }

  a.expr = parse_expr(p);
  
  if (!p->ok) {
    p->index = i;
    return a;
  }

  return a;
}

func_decl parse_func_decl(parser* p) {
  parse_whitespace(p);
  p->ok = 1;

  type type = parse_type(p);
  unit name = parse_text(p);
  
  parse_exact(p, '(');
  parse_exact(p, ')');

  block* block = parse_scope(p);

  func_decl f;
  f.name = name;
  f.ret = type;
  f.body = block;

  return f;
}


statement* parse_statement(parser* p) {
  int i = p->index;
  parse_whitespace(p);

  statement* s = malloc(sizeof(statement));

  // Check for if block
  p->ok = 1;
  s->kind = statement_if_kind;
  s->if_block = parse_if_block(p);
  if (p->ok) {
    return s;
  }

  // Check Check for function call
  p->ok = 1;
  s->kind = statement_call_kind;
  s->call = parse_call(p);
  parse_exact(p, ';');
  if (p->ok) {
    return s;
  }

  // Check assignment
  // @NOTE - I think there is a bug if this is not the last check here. Has to do with recovering..
  p->ok = 1;
  int ok = 1;
  s->kind = statement_assign_kind;
  s->assign = parse_assign(p);
  parse_exact(p, ';');
  if (ok) {
    return s;
  }

  return s;
}

block* parse_block(parser* p) {
  int i = p->index;


  block* b = malloc(sizeof(block));
  b->statement = parse_statement(p);
  parse_exact(p, ';');

  b->next = 0;

  block* iter = b;
  for (;;) {
    int i = p->index;
    int ok = 1;
    statement* s = parse_statement(p);

    if (!p->ok) {
      p->index = i;
      p->ok = 1; // This is an expected condition
      iter->next = 0;
      return b;
    }

    block* next = malloc(sizeof(block));
    next->statement = s;
    next->next = 0;
    iter->next = next;
    iter = next;
  }

  puts("FAILED PARSE BLOCK?");
  return b;
}

block* parse_scope(parser* p) {
  p->ok = 1;
  parse_exact(p, '{');
  block* b = parse_block(p);
  if (!p->ok) {
    return b;
  }
  parse_exact(p, '}');
  if (!p->ok) {
    return b;
  }

  return b;
}

if_block parse_if_block(parser* p) {
  int i = p->index; // Save index
  p->ok = 1;

  if_block ib;
  parse_exact(p, 'i');
  parse_exact(p, 'f');
  parse_exact(p, '(');
  parse_exact(p, ')');

  if (!p->ok) {
    p->index = i;
    return ib;
  }
  ib.body = parse_scope(p);
  return ib;
}
