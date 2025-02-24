
run:
	gcc main.c -g3 -std=c99 && ./a.out
san:
	gcc main.c -fsanitize=address -g3 -std=c99 && ./a.out

