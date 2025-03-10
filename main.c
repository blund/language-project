#include <stdio.h>

#include "ast.h"
#include "parse.h"
#include "print.h"

char* program =\
"\
int main() { \n\
  int a = 123;\n\
  epic();\n\
  if () {\n\
    if () {\n\
      int bla = 123;\n\
    }\n\
  }\n\
  int a = 456;\n\
  int a = epic();\n\
}";

int main() {
  parser p = {
    .code = program,
    .len = strlen(program),
    .index = 0,

    // This is for the printing after parsing
    .indent = 0,
  };

  func_decl f = parse_func_decl(&p);
  puts("------------------");
  puts("reconstructed code");
  puts("------------------");
  print_func_decl(&p, &f);

  puts("");
   
}
