/*  
	dbar - ascii percentage meter

	Copyright (c) 2007 by Robert Manea  <rob dot manea at gmail dot com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#include <stdio.h>
#include "dbar.h"

#define MAXLEN 512

int main(int argc, char *argv[]) {
  int i, nv;
  char aval[MAXLEN], *endptr;
  Dbar dbar;

  dbardefaults(&dbar, textual);
  
  for(i=1; i < argc; i++) {
    if(!strncmp(argv[i], "-w", 3)) {
      if(++i < argc)
	dbar.width = atoi(argv[i]);
    }
    else if(!strncmp(argv[i], "-s", 3)) {
      if(++i < argc)
	dbar.sym = argv[i][0];
    }
    else if(!strncmp(argv[i], "-max", 5)) {
      if(++i < argc) {
	dbar.maxval = strtod(argv[i], &endptr);
	if(*endptr) {
	  fprintf(stderr, "dbar: '%s' incorrect number format", argv[i]);
	  return EXIT_FAILURE;
	}
      }
    }
    else if(!strncmp(argv[i], "-min", 5)) {
      if(++i < argc) {
	dbar.minval = strtod(argv[i], &endptr);
	if(*endptr) {
	  fprintf(stderr, "dbar: '%s' incorrect number format", argv[i]);
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
      fprintf(stderr, "usage: dbar [-w <characters>] [-s <symbol>] [-min <minvalue>] [-max <maxvalue>] [-l <string>] [-nonl]\n");
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
