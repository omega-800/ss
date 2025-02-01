all: 
	gcc src/{,flexer/}*.c -std=c99 -o ss -Wall -Werror -fsanitize=address -g3 -lm
clean: 
	rm ss

