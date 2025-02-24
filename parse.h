#include <string.h>

#include "ast.h"

void parse_error(parser* p, int start);

void parse_whitespace(parser* p);
unit parse_text(parser* p);
value parse_number(parser* p);
void parse_exact(parser* p, char c);

value parse_string(parser* p);
type parse_type(parser* p);
call parse_call(parser* p);

expr* parse_expr(parser* p);
assign parse_assign(parser* p);
func_decl parse_func_decl(parser* p);

statement* parse_statement(parser* p);
block* parse_block(parser* p);
block* parse_scope(parser* p);
if_block parse_if_block(parser* p);
