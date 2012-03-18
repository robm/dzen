#include <stdio.h>
#include "dbar.h"

#define MAXLEN 1024

int main(int argc, char *argv[]) {
  int i, nv;
  char aval[MAXLEN], *endptr;
  Dbar dbar;

  dbardefaults(&dbar, graphical);

  for(i=1; i < argc; i++) {
    if(!strncmp(argv[i], "-w", 3)) {
      if(++i < argc)
	dbar.width = atoi(argv[i]);
    }
    else if(!strncmp(argv[i], "-h", 3)) {
      if(++i < argc)
	dbar.height = atoi(argv[i]);
    }
    else if(!strncmp(argv[i], "-sw", 4)) {
      if(++i < argc)
	dbar.segw = atoi(argv[i]);
    }
    else if(!strncmp(argv[i], "-sh", 4)) {
      if(++i < argc)
	dbar.segh = atoi(argv[i]);
    }
    else if(!strncmp(argv[i], "-ss", 4)) {
      if(++i < argc)
	dbar.segb = atoi(argv[i]);
    }
    else if(!strncmp(argv[i], "-s", 3)) {
      if(++i < argc) {
	switch(argv[i][0]) {
	case 'o':
	  dbar.style = outlined;
	  break;
	case 'v':
	  dbar.style = vertical;
	  break;
	case 'p':
	  dbar.style = pie;
	  break;
	default:
	  dbar.style = norm;
	  break;
	}
      }
    }
    else if(!strncmp(argv[i], "-fg", 4)) {
      if(++i < argc)
	dbar.fg = argv[i];
    }
    else if(!strncmp(argv[i], "-bg", 4)) {
      if(++i < argc)
	dbar.bg = argv[i];
    }
    else if(!strncmp(argv[i], "-max", 5)) {
      if(++i < argc) {
	dbar.maxval = strtod(argv[i], &endptr);
	if(*endptr) {
	  fprintf(stderr, "gdbar: '%s' incorrect number format", argv[i]);
	  return EXIT_FAILURE;
	}
      }
    }
    else if(!strncmp(argv[i], "-min", 5)) {
      if(++i < argc) {
	dbar.minval = strtod(argv[i], &endptr);
	if(*endptr) {
	  fprintf(stderr, "gdbar: '%s' incorrect number format", argv[i]);
	  return EXIT_FAILURE;
	}
      }
    }
    else if(!strncmp(argv[i], "-l", 3)) {
      if(++i < argc)
	dbar.label = argv[i];
    }
    else if(!strncmp(argv[i], "-nonl", 6)) {
      dbar.pnl = 0;
    }
    else {
      fprintf(stderr, "usage: gdbar [-s <o|v>] [-w <pixel>] [-h <pixel>] [-sw <pixel>] [-ss <pixel>] [-sw <pixel>] [-fg <color>] [-bg <color>] [-min <minvalue>] [-max <maxvalue>] [-l <string>] [-nonl] \n");
      return EXIT_FAILURE;
    }
  }

  while(fgets(aval, MAXLEN, stdin)) {
    nv = sscanf(aval, "%lf %lf %lf", &dbar.val, &dbar.minval, &dbar.maxval);
    if(nv == 2) {
      dbar.maxval = dbar.minval;
      dbar.minval = 0;
    }
    fdbar(&dbar, stdout);
  }

}
