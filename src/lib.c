#include "lib.h"
#include "sys/stat.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int is_space(const char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\a' || c == '\n' ||
         c == '\e' || c == '\f' || c == '\v';
}

int is_empty(const char *s) {
  while (*s != '\0') {
    if (!is_space((unsigned char)*s))
      return 0;
    s++;
  }
  return 1;
}

void free_list(struct ListItem *item) {
  item = item->first;
  do {
    struct ListItem *prev = item;
    item = item->next;
    free(prev->cur);
    free(prev);
  } while (item != item->last);
  free(item->cur);
  free(item);
}

struct ListItem *list_append(struct ListItem *list, char *value) {
  struct ListItem *append = malloc(sizeof(struct ListItem));
  append->cur = value;
  append->last = append;
  append->first = list->first;
  append->next = list->first;
  append->prev = list->last;

  list->last->next = append;
  list->last->last = append;
  list->first->prev = append;
  struct ListItem *cur = list->first;
  do {
    cur->last = append;
    cur = cur->next;
  } while (cur != list->first);
  return append; 
}

char *concat(const char *s1, const char *s2) {
  const size_t len1 = strlen(s1);
  const size_t len2 = strlen(s2);
  char *result = malloc(len1 + len2 + 1);
  memcpy(result, s1, len1);
  memcpy(result + len1, s2, len2 + 1);
  return result;
}

int getch(void) {
  int ch;
  struct termios oldt;
  struct termios newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return ch;
}

void err(const char *msg) {
  fprintf(stderr, "ss: %s\n", msg);
  exit(1);
}

FILE *open_or_die(char *path, const char *filename, const char *modes) {
  char *msg = malloc(128);
  if (mkdir(path, 0700) != 0 && EEXIST != errno) {
    snprintf(msg, 128, "error creating %s", path);
    err(msg);
  }
  char *filepath = malloc(128);
  snprintf(filepath, 128, "%s/%s", path, filename);
  FILE *file = fopen(filepath, modes);
  if (file == NULL) {
    snprintf(msg, 128, "error opening %s", filepath);
    err(msg);
  }
  free(msg);
  free(filepath);
  return file;
}

FILE *histfile() {
  const char *home = getenv("HOME");
  char *histd = concat(home, "/.local/state/ss");
  FILE *file = open_or_die(histd, "history", "a+");
  free(histd);
  return file;
}

FILE *conffile() {
  const char *home = getenv("HOME");
  char *confd = concat(home, "/.config/ss");
  FILE *file = open_or_die(confd, "ssrc", "a+");
  free(confd);
  return file;
}

#define RL_BUF 1024
struct ListItem *filelines(FILE *file) {
  if (!file) {
    err("no file");
    // making lsp happy
    exit(1);
  }

  struct ListItem *first = malloc(sizeof(struct ListItem));
  struct ListItem *cur = first;
  size_t len = RL_BUF * sizeof(char);
  size_t prev = 0;
  char *line = malloc(len);
  if (!line) {
    err("cannot allocate for history");
    exit(0);
  }
  line[0] = '\0';

  while (1) {
    if (fgets(&line[prev], len - prev, file) != NULL) {
      if (line[strlen(line) - 1] == '\n') {
        line[strlen(line) - 1] = '\0';
        cur->cur = line;
        cur->first = first;
        struct ListItem *next = malloc(sizeof(struct ListItem));
        cur->next = next;
        next->prev = cur;
        cur = next;
        prev = 0;
        len = RL_BUF * sizeof(char);
        line = malloc(len);
        continue;
      }

      prev = len;
      len += RL_BUF * sizeof(char);
      line = realloc(line, len);
      if (!line) {
        err("cannot allocate for history");
        exit(0);
      }
    } else if (feof(file)) {
      break;
    } else {
      err("error during histfile read");
      break;
    }
  }

  struct ListItem *last = cur->prev;
  cur = last;
  free(last->next);
  free(line);
  cur->next = first;
  first->prev = last;

  do {
    cur->last = last;
    cur = cur->next;
  } while (cur != last);

  return first;
}
