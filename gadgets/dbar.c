#include "dbar.h"

void
dbardefaults(Dbar *dbar, int mode) {
	dbar->bg	= "darkgrey";
	dbar->fg	= "white";
	dbar->label	= NULL;
	dbar->sym	= '=';
	dbar->val	= 0;
	dbar->minval	= 0;
	dbar->maxval	= 100.0;
	dbar->mode	= mode ? graphical : textual;
	dbar->style	= norm;
	dbar->width	= mode ? 80 : 25;
	dbar->height	= 10;
	dbar->segw	= 6;
	dbar->segh	= 2;
	dbar->segb	= 0;
	dbar->gs	= 0;
	dbar->gw	= 1;
	dbar->gc	= 0;
	dbar->pnl	= 1;
	memset(dbar->gb, '\0', MAX_GRAPH_VALS);
}

void
fdbar(Dbar *dbar, FILE *stream) {
	int i, rp, p, t;
	int segs, segsa;
	double l, perc;

	perc = (100 * (dbar->val - dbar->minval)) / (dbar->maxval - dbar->minval);

	switch(dbar->style) {
		case outlined:
			l = perc * ((double)(dbar->width-2) / 100);
			break;
		case vertical:
			l = perc * ((double)dbar->height / 100);
			break;
		case graph:
			l = perc * ((double)dbar->height / 100);
			break;
		default:
			l = perc * ((double)dbar->width / 100);
			break;
	}

	l=(int)(l + 0.5) >= (int)l ? l+0.5 : l;
	rp=(int)(perc + 0.5) >= (int)perc ? (int)(perc + 0.5) : (int)perc;

	if(dbar->mode == textual) {
		fprintf(stream, "%s%3d%% [", dbar->label ? dbar->label : "", rp);
		for(i=0; i < (int)l; i++)
			if(i == dbar->width) {
				fputc('>', stream);
				break;
			} else
				fputc(dbar->sym, stream);
			for(; i < dbar->width; i++)
				fputc(' ', stream);
			fprintf(stream, "]%s", dbar->pnl ? "\n" : "");
	} else {
		switch(dbar->style) {
			case outlined:
				if(dbar->segb == 0) {
					fprintf(stream, "%s^ib(1)^fg(%s)^ro(%dx%d)^p(%d)^fg(%s)^r(%dx%d)^p(%d)^ib(0)^fg()%s", 
							dbar->label ? dbar->label : "",
							dbar->bg, (int)dbar->width, dbar->height, -1*(dbar->width-2),
							dbar->fg, (int)l>dbar->width-4?dbar->width-4:(int)l, dbar->height-4>0?dbar->height-4:1,
							dbar->width-(int)l-1, dbar->pnl ? "\n" : "");
				} else {
					segs  = dbar->width / (dbar->segw + dbar->segb);
					segsa = rp * segs / 100;

					fprintf(stream, "%s^ib(1)^fg(%s)^ro(%dx%d)^p(%d)", 
							dbar->label ? dbar->label : "",
							dbar->bg, (int)dbar->width, dbar->height, -1*(dbar->width-2));
					for(i=0; i < segs; i++) {
						if(i<segsa)
							fprintf(stream, "^fg(%s)^r(%dx%d+%d+%d')", dbar->fg, dbar->segw, dbar->height-4>0?dbar->height-4:1, i?dbar->segb:0, 0);
						else
							break;
					}
					printf("^fg()^ib(0)^p(%d)%s", dbar->width - i*(dbar->segw + dbar->segb), dbar->pnl ? "\n" : "");
				}
				break;

			case vertical:
				segs  = dbar->height / (dbar->segh + dbar->segb);
				segsa = rp * segs / 100;
				fprintf(stream, "%s^ib(1)", dbar->label ? dbar->label : "");
				if(dbar->segb == 0) {
					//fprintf(stream, "^fg(%s)^r(%dx%d+%d-%d)^fg(%s)^p(-%d)^r(%dx%d+%d-%d)",
					//		dbar->bg, dbar->segw, dbar->height, 0, dbar->height+1,
					//		dbar->fg, dbar->segw, dbar->segw, (int)l, 0, (int)l+1);
					fprintf(stream, "^fg(%s)^r(%dx%d)^fg(%s)^r(%dx%d-%d+%d)",
							dbar->bg, dbar->segw, dbar->height,
							dbar->fg,  
							dbar->segw, (int)l, dbar->segw, (int)((dbar->height-l)/2.0 + .5));
				} else {
					for(i=0; i < segs; i++) {
						t = dbar->height/2-(dbar->segh+dbar->segb)*i;
						//if(i<segsa)
							fprintf(stream, "^fg(%s)^r(%dx%d-%d%c%d)",
									i < segsa ? dbar->fg : dbar->bg, dbar->segw, dbar->segh, 
									i?dbar->segw:0, t > 0 ? '+' : '-', abs(t));
							//fprintf(stream, "^fg(%s)^p(-%d)^r(%dx%d+%d-%d)",
							//		dbar->fg, i?dbar->segw:0, dbar->segw,
							//		dbar->segh, 0, (dbar->segh+dbar->segb)*(i+1));
						//else
							//fprintf(stream, "^fg(%s)^r(%dx%d-%d-%d)",
							//		dbar->bg, dbar->segw,
							//		dbar->segh,i?dbar->segw:0, (dbar->segh+dbar->segb)*i);
							//fprintf(stream, "^fg(%s)^p(-%d)^r(%dx%d+%d-%d)",
							//		dbar->bg, i?dbar->segw:0, dbar->segw,
							//		dbar->segh, 0, (dbar->segh+dbar->segb)*(i+1));
					}
				}
				fprintf(stream, "^ib(0)^fg()%s", dbar->pnl ? "\n" : "");
				break;

			case graph:
				dbar->gc = dbar->gc < MAX_GRAPH_VALS && 
					(dbar->gs == 0 ? dbar->gc : dbar->gc * dbar->gs + dbar->gc * dbar->gw)
					< dbar->width ? ++dbar->gc : 0;
				dbar->gb[dbar->gc] = l;

				printf("%s", dbar->label ? dbar->label : "");
				for(i=dbar->gc+1; i<MAX_GRAPH_VALS && (i*(dbar->gs+dbar->gw)) < dbar->width; ++i) {
					p=100*dbar->gb[i]/dbar->height;
					p=(int)p+0.5 >= (int)p ? (int)(p+0.5) : (int)p;
					fprintf(stream, "^fg(%s)^p(%d)^r(%dx%d+0-%d)", 
							dbar->fg, dbar->gs, dbar->gw, (int)dbar->gb[i], (int)dbar->gb[i]+1);
				}

				for(i=0; i < dbar->gc; ++i) {
					p=100*dbar->gb[i]/dbar->height;
					p=(int)p+0.5 >= (int)p ? (int)(p+0.5) : (int)p;
					fprintf(stream, "^fg(%s)^p(%d)^r(%dx%d+0-%d)", 
							dbar->fg, dbar->gs, dbar->gw, (int)dbar->gb[i], (int)dbar->gb[i]+1); 
				}
				fprintf(stream, "^fg()%s", dbar->pnl ? "\n" : "");
				break;

			case pie:
				fprintf(stream, "^ib(1)^fg(%s)^c(%d)^p(-%d)^fg(%s)^c(%d-%d)%s",
						dbar->bg, dbar->width, dbar->width, 
						dbar->fg, dbar->width, (int)(rp*360/100),
						dbar->pnl ? "\n" : "");
				break;

			default:
				if(dbar->segb == 0)
					//printf("%s%3d%% ^fg(%s)^r(%dx%d)^fg(%s)^r(%dx%d)^fg()%s", 
					printf("%s^fg(%s)^r(%dx%d)^fg(%s)^r(%dx%d)^fg()%s", 
							dbar->label ? dbar->label : "", 
							dbar->fg, (int)l, dbar->height,
							dbar->bg, dbar->width-(int)l, dbar->height,
							dbar->pnl ? "\n" : "");
				else {
					segs  = dbar->width / (dbar->segw + dbar->segb);
					segsa = rp * segs / 100;

					//printf("%s%3d%% ", dbar->label ? dbar->label : "", rp);
					printf("%s", dbar->label ? dbar->label : "");
					for(i=0; i < segs; i++) {
						if(i<segsa)
							fprintf(stream, "^fg(%s)^r(%dx%d+%d+%d)",
									dbar->fg, dbar->segw, dbar->height, i?dbar->segb:0, 0);
						else
							fprintf(stream, "^fg(%s)^r(%dx%d+%d+%d)",
									dbar->bg, dbar->segw, dbar->height, i?dbar->segb:0, 0);
					}
					fprintf(stream, "^p(%d)^fg()%s", dbar->segb, dbar->pnl ? "\n" : "");
				}
				break;
		}
	}
	fflush(stream);
}
