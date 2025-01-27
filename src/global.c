#include "lib.h"

FILE *histf;
struct ListItem *history;
FILE *conff;

void init() {
  histf = histfile();
  history = filelines(histf);
  conff = conffile();
}

void destroy() {
  if (histf != NULL)
    fclose(histf);
  if (conff != NULL)
    fclose(conff);
  if (history)
    free_list(history);
}
