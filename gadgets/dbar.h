#ifndef __DBAR_H
#define __DBAR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_GRAPH_VALS 1024

typedef struct {
  const char *bg;
  const char *fg; 
  const char *label;
  char sym;
  double val;
  double minval;
  double maxval;
  int mode;
  int style;
  int width;
  int height;
  int segw;
  int segh;
  int segb;
  int gs;
  int gw;
  int gc;
  char gb[MAX_GRAPH_VALS];
  int pnl;
} Dbar; 

enum mode  { textual, graphical };
enum style { norm, outlined, vertical, graph, pie };

void fdbar(Dbar *dbar, FILE *stream);
void dbardefaults(Dbar *dbar, int mode);

#endif /* __DBAR_H */
