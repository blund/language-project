

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>

#define THIS p->code[p->index]
#define DBG printf("%s\n", &THIS)
#define OK = p->ok

typedef enum kind {
  type_kind,
  assign_kind,
  call_kind,
  value_kind,
} kind;

typedef enum value_type {
  string_type,
  integer_type,
} value_type;

typedef struct parser {
  char* code;
  int len;
  int index;
  int ok;
} parser;

typedef struct node node;

typedef struct unit {
  int start; // Index of the first char of the unit
  int end;   // Index of the char following the unit
} unit;

typedef struct value {
  unit data;
  value_type type;
} value;

typedef struct type {
  unit name;
  int ptr; // 1 or 0
} type;

typedef struct call {
  unit name;
} call;

typedef struct block {
  node* statement;
  struct block* next; // Linked list, assume 0 == end
} block;

typedef struct func_decl {
  unit name;
  type ret;
  unit params;
  block* body;
} func_decl;

typedef struct assign {
  type type;
  unit name;
  node* expr;
} assign;

typedef struct node {
  kind kind;
  union {
    type type;
    assign assign;
    call call;
    value value;
  };
} node;


block* parse_scope(parser* p);
node* parse_statement(parser* p);
void parse_function(parser* p);

//return 0; \n					\

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

node parse_number(parser* p) {
  parse_whitespace(p);
  p->ok = 1; // Reset index
  int i = p->index; // Save index

  node n = {.kind = value_kind};
  value* v = &n.value;
  v->type = integer_type;
  
  unit u = {.start = p->index};

  // Return early if not ok
  if (THIS < '0' || THIS > '9') {
    p->ok = 0;
    return n;
  }
  
  while (THIS >= '0' && THIS <= '9'){
    p->index++;
  }

  u.end = p->index;
  v->data = u;
  return n;
}

void parse_exact(parser* p, char c) {
  parse_whitespace(p);
  p->ok = THIS == c;
  if (p->ok) p->index++;

  return;
}

node parse_string(parser* p) {
  parse_whitespace(p);
  p->ok = 1; // Reset index
  int i = p->index; // Save index

  node n = {.kind = value_kind};
  value* v = &n.value;
  v->type = string_type;
  
  unit u = {.start = p->index};

  // Return early if not ok

  parse_exact(p, '"');
  while (THIS != '"') {
    p->index++;
  }
  parse_exact(p, '"');

  u.end = p->index;
  v->data = u;
  return n;
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

unit parse_until(parser* p, char c) {
  parse_whitespace(p);
  unit u = {.start = p->index};

  while (THIS != c) p->index++;

  u.end = p->index;

  return u;
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

void print_expr(parser* p, node* n) {
  if (n->kind == value_kind) print_value(p, n->value);
  if (n->kind == call_kind)  print_call(p,  n->call);
}

void print_node(parser* p, node* n) {
  if (n->kind == assign_kind) print_assign(p, n->assign);
  if (n->kind == call_kind)   print_call(p,   n->call);

  printf(";\n");
}

void print_assign(parser* p, assign a) {

  print_type(p, a.type);
  printf(" ");
  print_unit(p, a.name);
  printf(" = ");
  print_expr(p, a.expr);
}

node parse_call(parser* p) {
  parse_whitespace(p);
  p->ok = 1;

  node n = {.kind = call_kind};
  call* c = &n.call;

  c->name = parse_text(p);
  if (!p->ok) return n;
  parse_exact(p, '(');
  if (!p->ok) return n;
  parse_exact(p, ')');
  if (!p->ok) return n;
  
  return n;
}

node parse_expr(parser* p) {
  parse_whitespace(p);
  p->ok = 1;
  node n;
  n = parse_call(p);
  if (!p->ok) n = parse_number(p);
  if (!p->ok) n = parse_string(p);
  return n;
}

node parse_assign(parser* p) {
  parse_whitespace(p);
  p->ok = 1;
  int i = p->index;

  node n = {.kind = assign_kind};
  assign a = n.assign;


  type type = parse_type(p);

  unit name = parse_text(p);

  parse_exact(p, '=');
  if (!p->ok) {
    p->index = i;
    return n;
  }

  node* expr_node = malloc(sizeof(node));
  *expr_node = parse_expr(p);
  
  if (!p->ok) {
    p->index = i;
    return n;
  }

  n.assign = (assign){
    .type = type,
    .name = name,
    .expr = expr_node,
  };

  return n;
}

func_decl* parse_function_decl(parser* p) {
  parse_whitespace(p);
  p->ok = 1;

  type type = parse_type(p);
  unit name = parse_text(p);
  
  parse_exact(p, '(');
  parse_exact(p, ')');

  block* block = parse_scope(p);

  func_decl* f = malloc(sizeof(func_decl));
  f->name = name;
  f->ret = type;
  f->body = block;

  return f;
}

void print_block(parser* p, block* b) {
 block* iter = b;

 // @TODO unsure about this logic, but it works B)
  for(;;) {
    if (iter->next != 0) {
      print_node(p, iter->statement);
      iter = iter->next;
    } else break;
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

node* parse_statement(parser* p) {
  parse_whitespace(p);

  node* n = malloc(sizeof(node));
  
  // Check assignment
  p->ok = 1;
  *n = parse_assign(p);
  if (p->ok) {
    return n;
  }

  // Check Check for function call
  p->ok = 1;
  *n = parse_call(p);
  if (p->ok) {
    return n;
  }

  return n;
}

block* parse_block(parser* p) {
  int i = p->index;

  block* b = malloc(sizeof(block));

  // Parse first statement
  b->statement = parse_statement(p);
  parse_exact(p, ';');

  // Return on failure
  if (!p->ok) {
    b->next = 0; // Indicate the end of linked list
    p->index = i;
    return b;
  }

  b->next = parse_block(p);

  return b;
}

block* parse_scope(parser* p) {
  p->ok = 1;
  parse_exact(p, '{');
  block* b = parse_block(p);
  parse_exact(p, '}');

  if (!p->ok) {
    puts("error");
  }
  return b;
}

char* program = \
"int main() { \n\
  int a = 123; \n\
  char* b = \"schmak123\"; \n\
  epic(); \n\
  int bebi = epic(); \n\
}";

int main() {
  int index = 0;

  parser p = {
    .code = program,
    .len = strlen(program),
    .index = 0,
  };

  func_decl* f = parse_function_decl(&p);
  print_func_decl(&p, f);
}
