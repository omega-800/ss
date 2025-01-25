#define BUILTINS_LEN 5
extern char *ss_builtins[BUILTINS_LEN];
extern int (*ss_fn[BUILTINS_LEN])(char **);
int ss_exec(char **);
