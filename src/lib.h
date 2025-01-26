#include <stdio.h>

int is_space(const char c); 
int is_quote(const char c); 
int is_empty(const char *s); 
void cut(char *str, size_t size);
void append(char *str, char c);
char *restrcpy(char *dest, const char *src);
char *insertat(char *str, size_t pos, char c);
char *removeat(char *str, size_t pos);
char *concat(const char *s1, const char *s2);
int getch(void);
void err(const char *msg);
FILE *open_or_die(char *path, const char *filename, const char *modes);
struct ListItem {
  struct ListItem *first;
  struct ListItem *next;
  struct ListItem *prev;
  char *cur;
};
void free_list(struct ListItem *item);
struct ListItem *list_append(struct ListItem *list, char *value);
FILE *histfile();
FILE *conffile();
struct ListItem *filelines(FILE *file);
