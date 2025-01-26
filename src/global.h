#include <stdio.h>

#define RL_BUF 64
extern FILE *histf;
extern struct ListItem *history;
extern FILE *conff;
void init();
void destroy();
