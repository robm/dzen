/*  
	gcpubar - graphical cpu usage bar, to be used with dzen

	Copyright (C) 2007 by Robert Manea, <rob dot manea at gmail dot com>

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

#include "dbar.h"

/* critical % value, color */
#define CPUCRIT 75
#define CRITCOL "#D56F6C"
/* medium % value, color */
#define CPUMED 50
#define MEDCOL "#EBA178"

struct cpu_info {
	unsigned long long user;
	unsigned long long sys;
	unsigned long long idle;
	unsigned long long iowait;
} ncpu, ocpu;

int main(int argc, char *argv[]) {
	int i, t;
	double total;
	struct cpu_info mcpu;
	FILE *statfp;
	char buf[256], *ep;
	Dbar dbar;

	int counts = 0;
	double ival = 1.0;

	dbardefaults(&dbar, graphical);
	dbar.mode = graphical;

	for(i=1; i < argc; i++) {
		if(!strncmp(argv[i], "-i", 3)) {
			if(i+1 < argc) {
				ival = strtod(argv[i+1], &ep);
				if(*ep) {
					fprintf(stderr, "%s: '-i'  Invalid interval value\n", argv[0]);
					return EXIT_FAILURE;
				}
				else
					i++;
			}
		}
		else if(!strncmp(argv[i], "-s", 3)) {
			if(++i < argc) {
				switch(argv[i][0]) {
					case 'o':
						dbar.style = outlined;
						break;
					case 'g':
						dbar.style = graph;
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
		else if(!strncmp(argv[i], "-c", 3)) {
			if(++i < argc)
				counts = atoi(argv[i]);
		}
		else if(!strncmp(argv[i], "-gs", 4)) {
			if(++i < argc)
				dbar.gs = atoi(argv[i]);
		}
		else if(!strncmp(argv[i], "-gw", 4)) {
			if(++i < argc)
				dbar.gw = atoi(argv[i]);
		}
		else if(!strncmp(argv[i], "-w", 3)) {
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
		else if(!strncmp(argv[i], "-fg", 4)) {
			if(++i < argc)
				dbar.fg = argv[i];
		}
		else if(!strncmp(argv[i], "-bg", 4)) {
			if(++i < argc)
				dbar.bg = argv[i];
		}
		else if(!strncmp(argv[i], "-l", 3)) {
			if(++i < argc)
				dbar.label = argv[i];
		}
		else if(!strncmp(argv[i], "-nonl", 6)) {
			dbar.pnl = 0;
		}
		else {
			printf("usage: %s [-l <label>] [-i <interval>] [-c <count>] [-fg <color>] [-bg <color>] [-w <pixel>] [-h <pixel>] [-s <o|g|v>] [-sw <pixel>] [-sh <pixel>] [-ss <pixel>] [-gs <pixel>] [-gw <pixel>] [-nonl]\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	if(!(statfp = fopen("/proc/stat", "r"))) {
		printf("%s: error opening '/proc/stat'\n", argv[0]);
		return EXIT_FAILURE;
	}

	t = counts > 0 ? counts : 1;
	while(t) {
		rewind(statfp); 
		while(fgets(buf, sizeof buf, statfp)) {
			if(!strncmp(buf, "cpu ", 4)) {
				unsigned long long unice;
				double myload;
				/* linux >= 2.6 */
				if((sscanf(buf, "cpu %llu %llu %llu %llu %llu", 
								&ncpu.user, &unice, &ncpu.sys, &ncpu.idle, &ncpu.iowait)) < 5) {
					printf("%s: wrong field count in /proc/stat\n", argv[0]);
					return EXIT_FAILURE;
				}
				ncpu.user += unice;

				mcpu.user   = ncpu.user - ocpu.user;
				mcpu.sys = ncpu.sys - ocpu.sys;
				mcpu.idle   = ncpu.idle - ocpu.idle;
				mcpu.iowait = ncpu.iowait - ocpu.iowait;

				dbar.maxval  = mcpu.user + mcpu.sys + mcpu.idle + mcpu.iowait;
				dbar.val = mcpu.user + mcpu.sys + mcpu.iowait;

				fdbar(&dbar, stdout);
				ocpu = ncpu;
			}
		}
		if((counts > 0) && (t-1 > 0)) {
			--t;
			usleep((unsigned long)(ival * 1000000.0));
		}
		else if((counts == 0) && (t == 1))
			usleep((unsigned long)(ival * 1000000.0));
		else 
			break;
	}

	return EXIT_SUCCESS;
}
