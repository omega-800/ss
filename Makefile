all: 
	gcc src/*.c -std=c99 -o ss -Wall -Werror -fsanitize=address -g3 
