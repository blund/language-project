
run:
	gcc main.c -fsanitize=address -g3 -std=c99 && ./a.out
