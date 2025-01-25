all: 
	gcc src/* -std=c99 -o ss -Wall -Werror -fsanitize=address -g3 
