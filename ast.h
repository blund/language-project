#ifndef AST_H
#define AST_H

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

  int indent;
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

#endif
