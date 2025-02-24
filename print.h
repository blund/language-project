#include "ast.h"

void print_unit(parser* p, unit u);
void print_type(parser* p, type t);
void print_value(parser* p, value v);
void print_call(parser* p, call c);
void print_if_block(parser* p, if_block ib);
void print_expr(parser* p, expr* e);
void print_statement(parser* p, statement* s);
void print_assign(parser* p, assign a);
void print_block(parser* p, block* b);
void print_func_decl(parser* p, func_decl* f);
