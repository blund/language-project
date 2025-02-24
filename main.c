#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#define THIS p->code[p->index]
#define DBG printf("DBG:\n%s\n\n", &THIS)
#define OK = p->ok

typedef enum value_type {
  string_type,
  integer_type,
} value_type;

typedef struct parser {
  char* code;
  int   len;
  int   index;
  int   ok;
} parser;

typedef struct statement statement;

typedef struct unit {
  int start; // Index of the first char of the unit
  int end;   // Index of the char following the unit
} unit;

typedef struct value {
  unit       data;
  value_type type;
} value;

typedef struct type {
  unit name;
  int  ptr; // 1 or 0
} type;

typedef struct call {
  unit name;
} call;

typedef enum statement_kind {
  statement_assign_kind,
  statement_call_kind,
  statement_if_kind,
} statement_kind;

typedef struct block {
  statement* statement;
  struct block* next; // Linked list, assume 0 == end
} block;

typedef struct if_block {
  unit condition;
  block* body;
} if_block;

typedef struct func_decl {
  unit   name;
  type   ret;
  unit   params;
  block* body;
} func_decl;

typedef enum expr_kind {
  expr_call_kind,
  expr_value_kind,
} expr_kind;

typedef struct expr {
  expr_kind kind;
  union {
    call  call;
    value value;
  };
} expr;

typedef struct assign {
  type  type;
  unit  name;
  expr* expr;
} assign;

typedef struct statement {
  statement_kind kind;
  union {
    assign   assign;
    call     call;
    if_block if_block;
  };
} statement;

block* parse_scope(parser* p);
statement* parse_statement(parser* p);
if_block parse_if_block(parser* p);

void print_unit(parser* p, unit u) {
  for (int i = u.start; i < u.end; i++) {
    printf("%c", p->code[i]);
  }
}

void parse_whitespace(parser* p) {
  if (p->index >= p->len) return;
  while (strchr(" \\\n", THIS)) p->index++;
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

void print_expr(parser* p, expr* e) {
  if (e->kind == expr_value_kind) print_value(p, e->value);
  if (e->kind == expr_call_kind)  print_call(p,  e->call);
}

void print_statement(parser* p, statement* s) {
  if (s->kind == statement_assign_kind) print_assign(p, s->assign);
  if (s->kind == statement_call_kind)   print_call(p,   s->call);
  if (s->kind == statement_if_kind)     puts("if-statement :)");

  printf(";\n");
}

void print_assign(parser* p, assign a) {
  print_type(p, a.type);
  printf(" ");
  print_unit(p, a.name);
  printf(" = ");
  print_expr(p, a.expr);
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

void print_block(parser* p, block* b) {
 block* iter = b;

 // @TODO unsure about this logic, but it works B)
  for(;;) {
    print_statement(p, iter->statement);
    if (iter->next == 0) return;
    iter = iter->next;
  }
}

void print_func_decl(parser* p, func_decl* f) {
  print_type(p,  f->ret);
  printf(" ");
  print_unit(p,  f->name);
  printf("() {\n");
  print_block(p, f->body);
  printf("}\n");
}

statement* parse_statement(parser* p) {
  parse_whitespace(p);

  statement* s = malloc(sizeof(statement));
 
  // Check assignment
  p->ok = 1;
  s->kind = statement_assign_kind;
  s->assign = parse_assign(p);
  if (p->ok) {
    return s;
  }

  // Check Check for function call
  p->ok = 1;
  s->kind = statement_call_kind;
  s->call = parse_call(p);
  if (p->ok) {
    return s;
  }

  /*
  // Check for if block
  p->ok = 1;
  s->kind = statement_if_kind;
  s->if_block = parse_if_block(p);
  if (p->ok) {
    return s;
  }
  */
  return s;
}

block* parse_block(parser* p) {
  int i = p->index;


  block* b = malloc(sizeof(block));
  b->statement = parse_statement(p);
  parse_exact(p, ';');

  if (!p->ok) return b;

  b->next = 0;

  block* iter = b;
  for (;;) {
    statement* s = parse_statement(p);
    parse_exact(p, ';');

    if (!p->ok) {
      p->ok = 1; // This is an expected condition
      iter->next = 0;
      return b;
    }

    block* next = malloc(sizeof(block));
    next->statement = s;
    iter->next = next;
    iter = next;
  }
 
  return b;
}

block* parse_scope(parser* p) {
  int i = p->index;
  p->ok = 1;
  parse_exact(p, '{');
  block* b = parse_block(p);
  parse_exact(p, '}');

  if (!p->ok) {
    p->index = 1;
    puts("error");
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

void parse_declaration(parser* p) {
  // Function declaration
  // Var declaration
}

void parse_top(parser* p) {
  // Declare some var e. Linked list of top level declarations
}

char* program = \
"\
int main() { \n\
  int a = 123; \n\
  char* b = \"schmak123\"; \n\
  epic(); \n\
  int bebi = epic(); \n\
}";


/*
 */

int main() {
  parser p = {
    .code = program,
    .len = strlen(program),
    .index = 0,
  };

  func_decl f = parse_func_decl(&p);
  print_func_decl(&p, &f);
}
