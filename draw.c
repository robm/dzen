
/*
* (C)opyright 2007-2009 Robert Manea <rob dot manea at gmail dot com>
* See LICENSE file for license details.
*
*/

#include "dzen.h"
#include "action.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef DZEN_XPM
#include <X11/xpm.h>
#endif

#define ARGLEN 256
#define MAX_ICON_CACHE 32

#define MAX(a,b) ((a)>(b)?(a):(b))
#define LNR2WINDOW(lnr) lnr==-1?0:1

typedef struct ICON_C {
	char name[ARGLEN];
	Pixmap p;

	int w, h;
} icon_c;

icon_c icons[MAX_ICON_CACHE];
int icon_cnt;
int otx;

int xorig[2];
sens_w window_sens[2];

/* command types for the in-text parser */
enum ctype  {bg, fg, icon, rect, recto, circle, circleo, pos, abspos, titlewin, ibg, fn, fixpos, ca, ba};

struct command_lookup {
	const char *name;
	int id;
	int off;
};

struct command_lookup cmd_lookup_table[] = {
	{ "fg(",        fg,			3},
	{ "bg(",        bg,			3},
	{ "i(",			icon,		2},
	{ "r(",	        rect,		2},
	{ "ro(",        recto,		3},
	{ "c(",	        circle,		2},
	{ "co(",        circleo,	3},
	{ "p(",	        pos,		2},
	{ "pa(",        abspos,		3},
	{ "tw(",        titlewin,	3},
	{ "ib(",        ibg,		3},
	{ "fn(",        fn,			3},
	{ "ca(",        ca,			3},
	{ "ba(",		ba,			3},
	{ 0,			0,			0}
};


/* positioning helpers */
enum sctype {LOCK_X, UNLOCK_X, TOP, BOTTOM, CENTER, LEFT, RIGHT};

int get_tokval(const char* line, char **retdata);
int get_token(const char*  line, int * t, char **tval);

static unsigned int
textnw(Fnt *font, const char *text, unsigned int len) {
#ifndef DZEN_XFT
	XRectangle r;

	if(font->set) {
		XmbTextExtents(font->set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(font->xfont, text, len);
#else
	XftTextExtentsUtf8(dzen.dpy, dzen.font.xftfont, (unsigned const char *) text, strlen(text), dzen.font.extents);
	if(dzen.font.extents->height > dzen.font.height)
		dzen.font.height = dzen.font.extents->height;
	return dzen.font.extents->xOff;
#endif
}


void
drawtext(const char *text, int reverse, int line, int align) {
	if(!reverse) {
		XSetForeground(dzen.dpy, dzen.gc, dzen.norm[ColBG]);
		XFillRectangle(dzen.dpy, dzen.slave_win.drawable[line], dzen.gc, 0, 0, dzen.w, dzen.h);
		XSetForeground(dzen.dpy, dzen.gc, dzen.norm[ColFG]);
	}
	else {
		XSetForeground(dzen.dpy, dzen.rgc, dzen.norm[ColFG]);
		XFillRectangle(dzen.dpy, dzen.slave_win.drawable[line], dzen.rgc, 0, 0, dzen.w, dzen.h);
		XSetForeground(dzen.dpy, dzen.rgc, dzen.norm[ColBG]);
	}

	parse_line(text, line, align, reverse, 0);
}

long
getcolor(const char *colstr) {
	Colormap cmap = DefaultColormap(dzen.dpy, dzen.screen);
	XColor color;

	if(!XAllocNamedColor(dzen.dpy, cmap, colstr, &color, &color))
		return -1;

	return color.pixel;
}

void
setfont(const char *fontstr) {
#ifndef DZEN_XFT
	char *def, **missing;
	int i, n;

	missing = NULL;
	if(dzen.font.set)
		XFreeFontSet(dzen.dpy, dzen.font.set);

	dzen.font.set = XCreateFontSet(dzen.dpy, fontstr, &missing, &n, &def);
	if(missing)
		XFreeStringList(missing);

	if(dzen.font.set) {
		XFontSetExtents *font_extents;
		XFontStruct **xfonts;
		char **font_names;
		dzen.font.ascent = dzen.font.descent = 0;
		font_extents = XExtentsOfFontSet(dzen.font.set);
		n = XFontsOfFontSet(dzen.font.set, &xfonts, &font_names);
		for(i = 0, dzen.font.ascent = 0, dzen.font.descent = 0; i < n; i++) {
			if(dzen.font.ascent < (*xfonts)->ascent)
				dzen.font.ascent = (*xfonts)->ascent;
			if(dzen.font.descent < (*xfonts)->descent)
				dzen.font.descent = (*xfonts)->descent;
			xfonts++;
		}
	}
	else {
		if(dzen.font.xfont)
			XFreeFont(dzen.dpy, dzen.font.xfont);
		dzen.font.xfont = NULL;
		if(!(dzen.font.xfont = XLoadQueryFont(dzen.dpy, fontstr)))
			eprint("dzen: error, cannot load font: '%s'\n", fontstr);
		dzen.font.ascent = dzen.font.xfont->ascent;
		dzen.font.descent = dzen.font.xfont->descent;
	}
	dzen.font.height = dzen.font.ascent + dzen.font.descent;
#else
        if(dzen.font.xftfont)
           XftFontClose(dzen.dpy, dzen.font.xftfont);
	dzen.font.xftfont = XftFontOpenXlfd(dzen.dpy, dzen.screen, fontstr);
	if(!dzen.font.xftfont)
	   dzen.font.xftfont = XftFontOpenName(dzen.dpy, dzen.screen, fontstr);
	if(!dzen.font.xftfont)
	   eprint("error, cannot load font: '%s'\n", fontstr);
	dzen.font.extents = malloc(sizeof(XGlyphInfo));
	XftTextExtentsUtf8(dzen.dpy, dzen.font.xftfont, (unsigned const char *) fontstr, strlen(fontstr), dzen.font.extents);
	dzen.font.height = dzen.font.xftfont->ascent + dzen.font.xftfont->descent;
	dzen.font.width = (dzen.font.extents->width)/strlen(fontstr);
#endif
}


int
get_tokval(const char* line, char **retdata) {
	int i;
	char tokval[ARGLEN];

	for(i=0; i < ARGLEN && (*(line+i) != ')'); i++)
		tokval[i] = *(line+i);

	tokval[i] = '\0';
	*retdata = strdup(tokval);

	return i+1;
}

int
get_token(const char *line, int * t, char **tval) {
	int off=0, next_pos=0, i;
	char *tokval = NULL;

	if(*(line+1) == ESC_CHAR)
		return 0;
	line++;

	for(i=0; cmd_lookup_table[i].name; ++i) {
		if( off=cmd_lookup_table[i].off,
				!strncmp(line, cmd_lookup_table[i].name, off) ) {
			next_pos = get_tokval(line+off, &tokval);
			*t = cmd_lookup_table[i].id;
			break;
		}
	}


	*tval = tokval;
	return next_pos+off;
}

static void
setcolor(Drawable *pm, int x, int width, long tfg, long tbg, int reverse, int nobg) {

	if(nobg)
		return;

	XSetForeground(dzen.dpy, dzen.tgc, reverse ? tfg : tbg);
	XFillRectangle(dzen.dpy, *pm, dzen.tgc, x, 0, width, dzen.line_height);

	XSetForeground(dzen.dpy, dzen.tgc, reverse ? tbg : tfg);
	XSetBackground(dzen.dpy, dzen.tgc, reverse ? tfg : tbg);
}

int 
get_sens_area(char *s, int *b, char *cmd) {
	memset(cmd, 0, 1024);
    sscanf(s, "%5d", b);
    char *comma = strchr(s, ',');
    if (comma != NULL)
        strncpy(cmd, comma+1, 1024);

	return 0;
}

static int
get_rect_vals(char *s, int *w, int *h, int *x, int *y) {
	*w=*h=*x=*y=0;

	return sscanf(s, "%5dx%5d%5d%5d", w, h, x, y);
}

static int
get_circle_vals(char *s, int *d, int *a) {
	int ret;
	*d=*a=ret=0;

	return  sscanf(s, "%5d%5d", d, a);
}

static int
get_pos_vals(char *s, int *d, int *a) {
	int i=0, ret=3, onlyx=1;
	char buf[128];
	*d=*a=0;

	if(s[0] == '_') {
		if(!strncmp(s, "_LOCK_X", 7)) {
			*d = LOCK_X;
		}
		if(!strncmp(s, "_UNLOCK_X", 8)) {
			*d = UNLOCK_X;
		}
		if(!strncmp(s, "_LEFT", 5)) {
			*d = LEFT;
		}
		if(!strncmp(s, "_RIGHT", 6)) {
			*d = RIGHT;
		}
		if(!strncmp(s, "_CENTER", 7)) {
			*d = CENTER;
		}
		if(!strncmp(s, "_BOTTOM", 7)) {
			*d = BOTTOM;
		}
		if(!strncmp(s, "_TOP", 4)) {
			*d = TOP;
		}

		return 5;
	} else {
		for(i=0; s[i] && i<128; i++) {
			if(s[i] == ';') {
				onlyx=0;
				break;
			} else
				buf[i]=s[i];
		}

		if(i) {
			buf[i]='\0';
			*d=atoi(buf);
		} else
			ret=2;

		if(s[++i]) {
			*a=atoi(s+i);
		} else
			ret = 1;

		if(onlyx) ret=1;

		return ret;
	}
}

static int
get_block_align_vals(char *s, int *a, int *w)
{
	char buf[32];
	int r;
	*w = -1;
	r = sscanf(s, "%d,%31s", w, buf);
	if(!strcmp(buf, "_LEFT"))
		*a = ALIGNLEFT;
	else if(!strcmp(buf, "_RIGHT"))
		*a = ALIGNRIGHT;
	else if(!strcmp(buf, "_CENTER"))
		*a = ALIGNCENTER;
	else
		*a = -1;

	return r;
}


static int
search_icon_cache(const char* name) {
	int i;

	for(i=0; i < MAX_ICON_CACHE; i++)
		if(!strncmp(icons[i].name, name, ARGLEN))
			return i;

	return -1;
}

#ifdef DZEN_XPM
static void
cache_icon(const char* name, Pixmap pm, int w, int h) {
	if(icon_cnt >= MAX_ICON_CACHE)
		icon_cnt = 0;

	if(icons[icon_cnt].p)
		XFreePixmap(dzen.dpy, icons[icon_cnt].p);

	strncpy(icons[icon_cnt].name, name, ARGLEN);
	icons[icon_cnt].w = w;
	icons[icon_cnt].h = h;
	icons[icon_cnt].p = pm;
	icon_cnt++;
}
#endif


char *
parse_line(const char *line, int lnr, int align, int reverse, int nodraw) {
	/* bitmaps */
	unsigned int bm_w, bm_h;
	int bm_xh, bm_yh;
	/* rectangles, cirlcles*/
	int rectw, recth, rectx, recty;
	/* positioning */
	int n_posx, n_posy, set_posy=0;
	int px=0, py=0, opx=0;
	int i, next_pos=0, j=0, h=0, tw=0;
	/* buffer pos */
	const char *linep=NULL;
	/* fonts */
	int font_was_set=0;
	/* position */
	int pos_is_fixed = 0;
	/* block alignment */
	int block_align = -1;
	int block_width = -1;
	/* clickable area y tracking */
	int max_y=-1;

	/* temp buffers */
	char lbuf[MAX_LINE_LEN], *rbuf = NULL;

	/* parser state */
	int t=-1, nobg=0;
	char *tval=NULL;

	/* X stuff */
	long lastfg = dzen.norm[ColFG], lastbg = dzen.norm[ColBG];
	Fnt *cur_fnt = NULL;
#ifndef DZEN_XFT
	XGCValues gcv;
#endif
	Drawable pm=0, bm;
#ifdef DZEN_XPM
	int free_xpm_attrib = 0;
	Pixmap xpm_pm;
	XpmAttributes xpma;
	XpmColorSymbol xpms;
#endif

#ifdef DZEN_XFT
	XftDraw *xftd=NULL;
	XftColor xftc;
	char *xftcs;
	int xftcs_f=0;
	char *xftcs_bg;
	int xftcs_bgf=0;

	xftcs    = (char *)dzen.fg;
    xftcs_bg = (char *)dzen.bg;
#endif

	/* icon cache */
	int ip;

	/* parse line and return the text without control commands */
	if(nodraw) {
		rbuf = emalloc(MAX_LINE_LEN);
		rbuf[0] = '\0';
		if( (lnr + dzen.slave_win.first_line_vis) >= dzen.slave_win.tcnt)
			line = NULL;
		else
			line = dzen.slave_win.tbuf[dzen.slave_win.first_line_vis+lnr];

	}
	/* parse line and render text */
	else {
		h = dzen.font.height;
		py = (dzen.line_height - h) / 2;
		xorig[LNR2WINDOW(lnr)] = 0;
		
		if(lnr != -1) {
			pm = XCreatePixmap(dzen.dpy, RootWindow(dzen.dpy, DefaultScreen(dzen.dpy)), dzen.slave_win.width,
					dzen.line_height, DefaultDepth(dzen.dpy, dzen.screen));
		}
		else {
			pm = XCreatePixmap(dzen.dpy, RootWindow(dzen.dpy, DefaultScreen(dzen.dpy)), dzen.title_win.width,
					dzen.line_height, DefaultDepth(dzen.dpy, dzen.screen));
		}

#ifdef DZEN_XFT
		xftd = XftDrawCreate(dzen.dpy, pm, DefaultVisual(dzen.dpy, dzen.screen), 
				DefaultColormap(dzen.dpy, dzen.screen));
#endif

		if(!reverse) {
			XSetForeground(dzen.dpy, dzen.tgc, dzen.norm[ColBG]);
#ifdef DZEN_XPM
			xpms.pixel = dzen.norm[ColBG];
#endif
#ifdef DZEN_XFT
			xftcs_bg = (char *)dzen.bg;
			xftcs_bgf = 0;
#endif
		}
		else {
			XSetForeground(dzen.dpy, dzen.tgc, dzen.norm[ColFG]);
#ifdef DZEN_XPM
			xpms.pixel = dzen.norm[ColFG];
#endif
		}
		XFillRectangle(dzen.dpy, pm, dzen.tgc, 0, 0, dzen.w, dzen.h);

		if(!reverse) {
			XSetForeground(dzen.dpy, dzen.tgc, dzen.norm[ColFG]);
		}
		else {
			XSetForeground(dzen.dpy, dzen.tgc, dzen.norm[ColBG]);
		}

#ifdef DZEN_XPM
		xpms.name = NULL;
		xpms.value = (char *)"none";

		xpma.colormap = DefaultColormap(dzen.dpy, dzen.screen);
		xpma.depth = DefaultDepth(dzen.dpy, dzen.screen);
		xpma.visual = DefaultVisual(dzen.dpy, dzen.screen);
		xpma.colorsymbols = &xpms;
		xpma.numsymbols = 1;
		xpma.valuemask = XpmColormap|XpmDepth|XpmVisual|XpmColorSymbols;
#endif

#ifndef DZEN_XFT 
		if(!dzen.font.set){
			gcv.font = dzen.font.xfont->fid;
			XChangeGC(dzen.dpy, dzen.tgc, GCFont, &gcv);
		}
#endif
		cur_fnt = &dzen.font;

		if( lnr != -1 && (lnr + dzen.slave_win.first_line_vis >= dzen.slave_win.tcnt)) {
			XCopyArea(dzen.dpy, pm, dzen.slave_win.drawable[lnr], dzen.gc,
					0, 0, px, dzen.line_height, xorig[LNR2WINDOW(lnr)], 0);
			XFreePixmap(dzen.dpy, pm);
			return NULL;
		}
	}

	linep = line;
	while(1) {
		if(*linep == ESC_CHAR || *linep == '\0') {
			lbuf[j] = '\0';

			/* clear _lock_x at EOL so final width is correct */
			if(*linep=='\0')
				pos_is_fixed=0;

			if(nodraw) {
				strcat(rbuf, lbuf);
			}
			else {
				if(t != -1 && tval) {
					switch(t) {
						case icon:
							if(MAX_ICON_CACHE && (ip=search_icon_cache(tval)) != -1) {
								int y;
								XCopyArea(dzen.dpy, icons[ip].p, pm, dzen.tgc,
										0, 0, icons[ip].w, icons[ip].h, px, y=(set_posy ? py :
										(dzen.line_height >= (signed)icons[ip].h ?
										(dzen.line_height - icons[ip].h)/2 : 0)));
								px += !pos_is_fixed ? icons[ip].w : 0;
								max_y = MAX(max_y, y+icons[ip].h);
							} else {
								int y;
								if(XReadBitmapFile(dzen.dpy, pm, tval, &bm_w,
											&bm_h, &bm, &bm_xh, &bm_yh) == BitmapSuccess
										&& (h/2 + px + (signed)bm_w < dzen.w)) {
									setcolor(&pm, px, bm_w, lastfg, lastbg, reverse, nobg);

									XCopyPlane(dzen.dpy, bm, pm, dzen.tgc,
											0, 0, bm_w, bm_h, px, y=(set_posy ? py :
											(dzen.line_height >= (int)bm_h ?
												(dzen.line_height - (int)bm_h)/2 : 0)), 1);
									XFreePixmap(dzen.dpy, bm);
									px += !pos_is_fixed ? bm_w : 0;
									max_y = MAX(max_y, y+bm_h);
								}
#ifdef DZEN_XPM
								else if(XpmReadFileToPixmap(dzen.dpy, dzen.title_win.win, tval, &xpm_pm, NULL, &xpma) == XpmSuccess) {
									setcolor(&pm, px, xpma.width, lastfg, lastbg, reverse, nobg);

									if(MAX_ICON_CACHE)
										cache_icon(tval, xpm_pm, xpma.width, xpma.height);

									XCopyArea(dzen.dpy, xpm_pm, pm, dzen.tgc,
											0, 0, xpma.width, xpma.height, px, y=(set_posy ? py :
											(dzen.line_height >= (int)xpma.height ?
												(dzen.line_height - (int)xpma.height)/2 : 0)));
									px += !pos_is_fixed ? xpma.width : 0;
									max_y = MAX(max_y, y+xpma.height);

									/* freed by cache_icon() */
									//XFreePixmap(dzen.dpy, xpm_pm);
									free_xpm_attrib = 1;
								}
#endif
							}
							break;


						case rect:
							get_rect_vals(tval, &rectw, &recth, &rectx, &recty);
							recth = recth > dzen.line_height ? dzen.line_height : recth;
							if(set_posy)
								py += recty;
							recty =	recty == 0 ? (dzen.line_height - recth)/2 :
								(dzen.line_height - recth)/2 + recty;
							px += !pos_is_fixed ? rectx : 0;
							setcolor(&pm, px, rectw, lastfg, lastbg, reverse, nobg);

							XFillRectangle(dzen.dpy, pm, dzen.tgc, px,
									set_posy ? py :
									((int)recty < 0 ? dzen.line_height + recty : recty),
									rectw, recth);

							px += !pos_is_fixed ? rectw : 0;
							break;

						case recto:
							get_rect_vals(tval, &rectw, &recth, &rectx, &recty);
							if (!rectw) break;

							recth = recth > dzen.line_height ? dzen.line_height-2 : recth-1;
							if(set_posy)
								py += recty;
							recty =	recty == 0 ? (dzen.line_height - recth)/2 :
								(dzen.line_height - recth)/2 + recty;
							px = (rectx == 0) ? px : rectx+px;
							/* prevent from stairs effect when rounding recty */
							if (!((dzen.line_height - recth) % 2)) recty--;
							setcolor(&pm, px, rectw, lastfg, lastbg, reverse, nobg);
							XDrawRectangle(dzen.dpy, pm, dzen.tgc, px,
									set_posy ? py :
									((int)recty<0 ? dzen.line_height + recty : recty), rectw-1, recth);
							px += !pos_is_fixed ? rectw : 0;
							break;

						case circle:
							rectx = get_circle_vals(tval, &rectw, &recth);
							setcolor(&pm, px, rectw, lastfg, lastbg, reverse, nobg);
							XFillArc(dzen.dpy, pm, dzen.tgc, px, set_posy ? py :(dzen.line_height - rectw)/2,
									rectw, rectw, 90*64, rectx>1?recth*64:64*360);
							px += !pos_is_fixed ? rectw : 0;
							break;

						case circleo:
							rectx = get_circle_vals(tval, &rectw, &recth);
							setcolor(&pm, px, rectw, lastfg, lastbg, reverse, nobg);
							XDrawArc(dzen.dpy, pm, dzen.tgc, px, set_posy ? py : (dzen.line_height - rectw)/2,
									rectw, rectw, 90*64, rectx>1?recth*64:64*360);
							px += !pos_is_fixed ? rectw : 0;
							break;

						case pos:
							if(tval[0]) {
								int r=0;
								r = get_pos_vals(tval, &n_posx, &n_posy);
								if( (r == 1 && !set_posy))
									set_posy=0;
								else if (r == 5) {
									switch(n_posx) {
										case LOCK_X:
											pos_is_fixed = 1;
											break;
										case UNLOCK_X:
											pos_is_fixed = 0;
											break;
										case LEFT:
											px = 0;
											break;
										case RIGHT:
											px = dzen.w;
											break;
										case CENTER:
											px = dzen.w/2;
											break;
										case BOTTOM:
											set_posy = 1;
											py = dzen.line_height;
											break;
										case TOP:
											set_posy = 1;
											py = 0;
											break;
									}
								} else
									set_posy=1;

								if(r != 2)
									px = px+n_posx<0? 0 : px + n_posx;
								if(r != 1) 
									py += n_posy;
							} else {
								set_posy = 0;
								py = (dzen.line_height - dzen.font.height) / 2;
							}
							break;

						case abspos:
							if(tval[0]) {
								int r=0;
								if( (r=get_pos_vals(tval, &n_posx, &n_posy)) == 1 && !set_posy)
									set_posy=0;
								else
									set_posy=1;

								n_posx = n_posx < 0 ? n_posx*-1 : n_posx;
								if(r != 2)
									px = n_posx;
								if(r != 1)
									py = n_posy;
							} else {
								set_posy = 0;
								py = (dzen.line_height - dzen.font.height) / 2;
							}
							break;

						case ibg:
							nobg = atoi(tval);
							break;

						case bg:
							lastbg = tval[0] ? (unsigned)getcolor(tval) : dzen.norm[ColBG];
#ifdef DZEN_XFT
							if(xftcs_bgf) free(xftcs_bg);				
							if(tval[0]) {
								xftcs_bg = estrdup(tval);
								xftcs_bgf = 1;
							} else {
								xftcs_bg = (char *)dzen.bg;
								xftcs_bgf = 0;
							}
#endif							

							break;

						case fg:
							lastfg = tval[0] ? (unsigned)getcolor(tval) : dzen.norm[ColFG];
							XSetForeground(dzen.dpy, dzen.tgc, lastfg);
#ifdef DZEN_XFT
							if(tval[0]) {
								xftcs = estrdup(tval);
								xftcs_f = 1;
							} else {
								xftcs = (char *)dzen.fg;
								xftcs_f = 0;
							}
#endif							
							break;

						case fn:
							if(tval[0]) {
#ifndef DZEN_XFT		
								if(!strncmp(tval, "dfnt", 4)) {
									cur_fnt = &(dzen.fnpl[atoi(tval+4)]);

									if(!cur_fnt->set) {
										gcv.font = cur_fnt->xfont->fid;
										XChangeGC(dzen.dpy, dzen.tgc, GCFont, &gcv);
									}
								}
								else
#endif					
									setfont(tval);
							}
							else {
								cur_fnt = &dzen.font;
#ifndef DZEN_XFT		
								if(!cur_fnt->set){
									gcv.font = cur_fnt->xfont->fid;
									XChangeGC(dzen.dpy, dzen.tgc, GCFont, &gcv);
								}
#else
							setfont(dzen.fnt ? dzen.fnt : FONT);
#endif								
							}
							py = set_posy ? py : (dzen.line_height - cur_fnt->height) / 2;
							font_was_set = 1;
							break;
						case ca:
							; //nop to keep gcc happy
							sens_w *w = &window_sens[LNR2WINDOW(lnr)];
							
							if(tval[0]) {
								click_a *area = &((*w).sens_areas[(*w).sens_areas_cnt]);
								if((*w).sens_areas_cnt < MAX_CLICKABLE_AREAS) {
									get_sens_area(tval, 
											&(*area).button, 
											(*area).cmd);
									(*area).start_x = px;
									(*area).start_y = py;
									(*area).end_y = py;
									max_y = py;
									(*area).active = 0;
									if(lnr == -1) {
										(*area).win = dzen.title_win.win;
									} else {
										(*area).win = dzen.slave_win.line[lnr];
									}
									(*w).sens_areas_cnt++;
								}
							} else {
									//find most recent unclosed area
									for(i = (*w).sens_areas_cnt - 1; i >= 0; i--)
										if(!(*w).sens_areas[i].active)
											break;
									if(i >= 0 && i < MAX_CLICKABLE_AREAS) {
										(*w).sens_areas[i].end_x = px;
										(*w).sens_areas[i].end_y = max_y;
										(*w).sens_areas[i].active = 1;
								}
							}
							break;
						case ba:
							if(tval[0])
								get_block_align_vals(tval, &block_align, &block_width);
							else
								block_align=block_width=-1;
							break;
					}
					free(tval);
				}

				/* check if text is longer than window's width */
				tw = textnw(cur_fnt, lbuf, strlen(lbuf));
				while((((tw + px) > (dzen.w)) || (block_align!=-1 && tw>block_width)) && j>=0) {
					lbuf[--j] = '\0';
					tw = textnw(cur_fnt, lbuf, strlen(lbuf));
				}
				
				opx = px;

				/* draw background for block */
				if(block_align!=-1 && !nobg) {
					setcolor(&pm, px, rectw, lastbg, lastbg, 0, nobg);
					XFillRectangle(dzen.dpy, pm, dzen.tgc, px, 0, block_width, dzen.line_height);
				}

				if(block_align==ALIGNRIGHT)
					px += (block_width - tw);
				else if(block_align==ALIGNCENTER)
					px += (block_width/2) - (tw/2);

				if(!nobg)
					setcolor(&pm, px, tw, lastfg, lastbg, reverse, nobg);
				
#ifndef DZEN_XFT
				if(cur_fnt->set)
					XmbDrawString(dzen.dpy, pm, cur_fnt->set,
							dzen.tgc, px, py + cur_fnt->ascent, lbuf, strlen(lbuf));
				else
					XDrawString(dzen.dpy, pm, dzen.tgc, px, py+dzen.font.ascent, lbuf, strlen(lbuf));
#else
				if(reverse) {
				XftColorAllocName(dzen.dpy, DefaultVisual(dzen.dpy, dzen.screen),
						DefaultColormap(dzen.dpy, dzen.screen),  xftcs_bg,  &xftc);
				} else {
				XftColorAllocName(dzen.dpy, DefaultVisual(dzen.dpy, dzen.screen),
						DefaultColormap(dzen.dpy, dzen.screen),  xftcs,  &xftc);
				}

				XftDrawStringUtf8(xftd, &xftc, 
						cur_fnt->xftfont, px, py + dzen.font.xftfont->ascent, (const FcChar8 *)lbuf, strlen(lbuf));

				if(xftcs_f) {
					free(xftcs);
					xftcs_f = 0;
				}
				if(xftcs_bgf) {
					free(xftcs_bg);
					xftcs_bgf = 0;
				}

#endif

				max_y = MAX(max_y, py+dzen.font.height);

				if(block_align==-1) {
					if(!pos_is_fixed || *linep =='\0')
						px += tw;
				} else {
					if(pos_is_fixed)
						px = opx;
					else
						px = opx+block_width;
				}

				block_align=block_width=-1;
			}

			if(*linep=='\0')
				break;

			j=0; t=-1; tval=NULL;
			next_pos = get_token(linep, &t, &tval);
			linep += next_pos;

			/* ^^ escapes */
			if(next_pos == 0)
				lbuf[j++] = *linep++;
		}
		else
			lbuf[j++] = *linep;

		linep++;
	}

	if(!nodraw) {
		/* expand/shrink dynamically */
		if(dzen.title_win.expand && lnr == -1){
			i = px;
			switch(dzen.title_win.expand) {
				case left:
					/* grow left end */
					otx = dzen.title_win.x_right_corner - i > dzen.title_win.x ?
						dzen.title_win.x_right_corner - i : dzen.title_win.x;
					XMoveResizeWindow(dzen.dpy, dzen.title_win.win, otx, dzen.title_win.y, px, dzen.line_height);
					break;
				case right:
					XResizeWindow(dzen.dpy, dzen.title_win.win, px, dzen.line_height);
					break;
			}

		} else {
			if(align == ALIGNLEFT)
				xorig[LNR2WINDOW(lnr)] = 0;
			if(align == ALIGNCENTER) {
				xorig[LNR2WINDOW(lnr)] = (lnr != -1) ?
					(dzen.slave_win.width - px)/2 :
					(dzen.title_win.width - px)/2;
			}
			else if(align == ALIGNRIGHT) {
				xorig[LNR2WINDOW(lnr)] = (lnr != -1) ?
					(dzen.slave_win.width - px) :
					(dzen.title_win.width - px);
			}
		}


		if(lnr != -1) {
			XCopyArea(dzen.dpy, pm, dzen.slave_win.drawable[lnr], dzen.gc,
                    0, 0, dzen.w, dzen.line_height, xorig[LNR2WINDOW(lnr)], 0);
		}
		else {
			XCopyArea(dzen.dpy, pm, dzen.title_win.drawable, dzen.gc,
					0, 0, dzen.w, dzen.line_height, xorig[LNR2WINDOW(lnr)], 0);
		}
		XFreePixmap(dzen.dpy, pm);

		/* reset font to default */
		if(font_was_set)
			setfont(dzen.fnt ? dzen.fnt : FONT);

#ifdef DZEN_XPM
		if(free_xpm_attrib) {
			XFreeColors(dzen.dpy, xpma.colormap, xpma.pixels, xpma.npixels, xpma.depth);
			XpmFreeAttributes(&xpma);
		}
#endif

#ifdef DZEN_XFT
		XftDrawDestroy(xftd);
#endif
	}

	return nodraw ? rbuf : NULL;
}

int
parse_non_drawing_commands(char * text) {

	if(!text)
		return 1;

	if(!strncmp(text, "^togglecollapse()", strlen("^togglecollapse()"))) {
		a_togglecollapse(NULL);
		return 0;
	}
	if(!strncmp(text, "^collapse()", strlen("^collapse()"))) {
		a_collapse(NULL);
		return 0;
	}
	if(!strncmp(text, "^uncollapse()", strlen("^uncollapse()"))) {
		a_uncollapse(NULL);
		return 0;
	}

	if(!strncmp(text, "^togglestick()", strlen("^togglestick()"))) {
		a_togglestick(NULL);
		return 0;
	}
	if(!strncmp(text, "^stick()", strlen("^stick()"))) {
		a_stick(NULL);
		return 0;
	}
	if(!strncmp(text, "^unstick()", strlen("^unstick()"))) {
		a_unstick(NULL);
		return 0;
	}

	if(!strncmp(text, "^togglehide()", strlen("^togglehide()"))) {
		a_togglehide(NULL);
		return 0;
	}
	if(!strncmp(text, "^hide()", strlen("^hide()"))) {
		a_hide(NULL);
		return 0;
	}
	if(!strncmp(text, "^unhide()", strlen("^unhide()"))) {
		a_unhide(NULL);
		return 0;
	}

	if(!strncmp(text, "^raise()", strlen("^raise()"))) {
		a_raise(NULL);
		return 0;
	}

	if(!strncmp(text, "^lower()", strlen("^lower()"))) {
		a_lower(NULL);
		return 0;
	}

	if(!strncmp(text, "^scrollhome()", strlen("^scrollhome()"))) {
		a_scrollhome(NULL);
		return 0;
	}

	if(!strncmp(text, "^scrollend()", strlen("^scrollend()"))) {
		a_scrollend(NULL);
		return 0;
	}

	if(!strncmp(text, "^exit()", strlen("^exit()"))) {
		a_exit(NULL);
		return 0;
	}

	return 1;
}


void
drawheader(const char * text) {
	if(parse_non_drawing_commands((char *)text)) {
		if (text){
			dzen.w = dzen.title_win.width;
			dzen.h = dzen.line_height;
			
			window_sens[TOPWINDOW].sens_areas_cnt = 0;
			
			XFillRectangle(dzen.dpy, dzen.title_win.drawable, dzen.rgc, 0, 0, dzen.w, dzen.h);
			parse_line(text, -1, dzen.title_win.alignment, 0, 0);
		}
	} else {
		dzen.slave_win.tcnt = -1;
		dzen.cur_line = 0;
	}

	XCopyArea(dzen.dpy, dzen.title_win.drawable, dzen.title_win.win,
			dzen.gc, 0, 0, dzen.title_win.width, dzen.line_height, 0, 0);
}

void
drawbody(char * text) {
	char *ec;
	int i, write_buffer=1;
	

	if(dzen.slave_win.tcnt == -1) {
		dzen.slave_win.tcnt = 0;
		drawheader(text);
		return;
	}

	
	if((ec = strstr(text, "^tw()")) && (*(ec-1) != '^')) {
		drawheader(ec+5);
		return;
	}

	if(dzen.slave_win.tcnt == dzen.slave_win.tsize)
		free_buffer();

	write_buffer = parse_non_drawing_commands(text);


	if(text[0] == '^' && text[1] == 'c' && text[2] == 's') {
		free_buffer();

		for(i=0; i < dzen.slave_win.max_lines; i++)
			XFillRectangle(dzen.dpy, dzen.slave_win.drawable[i], dzen.rgc, 0, 0, dzen.slave_win.width, dzen.line_height);
		x_draw_body();
		return;
	}

	if( write_buffer && (dzen.slave_win.tcnt < dzen.slave_win.tsize) ) {
		dzen.slave_win.tbuf[dzen.slave_win.tcnt] = estrdup(text);
		dzen.slave_win.tcnt++;
	}
}
