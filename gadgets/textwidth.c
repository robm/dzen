/*
    textwidth - calculate width in pixels of text with a given font

    Copyright (C) 2007 by Robert Manea  <rob dot manea at gmail dot com>

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


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<X11/Xlib.h>

typedef struct _Fnt {
	XFontStruct *xfont;
	XFontSet set;
	int ascent;
	int descent;
	int height;
} Fnt;

Fnt font;
Display *dpy;

unsigned int
textw(const char *text, unsigned int len) {
	XRectangle r;

	if(font.set) {
		XmbTextExtents(font.set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(font.xfont, text, len);
}

void
setfont(const char *fontstr) {
	char *def, **missing;
	int i, n;

	missing = NULL;
	if(font.set)
		XFreeFontSet(dpy, font.set);
	font.set = XCreateFontSet(dpy, fontstr, &missing, &n, &def);
	if(missing)
		XFreeStringList(missing);
	if(font.set) {
		XFontSetExtents *font_extents;
		XFontStruct **xfonts;
		char **font_names;
		font.ascent = font.descent = 0;
		font_extents = XExtentsOfFontSet(font.set);
		n = XFontsOfFontSet(font.set, &xfonts, &font_names);
		for(i = 0, font.ascent = 0, font.descent = 0; i < n; i++) {
			if(font.ascent < (*xfonts)->ascent)
				font.ascent = (*xfonts)->ascent;
			if(font.descent < (*xfonts)->descent)
				font.descent = (*xfonts)->descent;
			xfonts++;
		}
	}
	else {
		if(font.xfont)
			XFreeFont(dpy, font.xfont);
		font.xfont = NULL;
		if(!(font.xfont = XLoadQueryFont(dpy, fontstr))) {
			fprintf(stderr, "error, cannot load font: '%s'\n", fontstr);
			exit(EXIT_FAILURE);
		}
		font.ascent = font.xfont->ascent;
		font.descent = font.xfont->descent;
	}
	font.height = font.ascent + font.descent;
}

int
main(int argc, char *argv[])
{
	char *myfont, *text;

	if(argc < 3) {
		fprintf(stderr, "usage: %s <font> <string>\n", argv[0]);
		return EXIT_FAILURE;
	}

	myfont = argv[1];
	text   = argv[2];

	dpy = XOpenDisplay(0);
	if(!dpy) {
		fprintf(stderr, "cannot open display\n");
		return EXIT_FAILURE;
	}

	setfont(myfont);
	printf("%u\n", textw(text, strlen(text)));

	return EXIT_SUCCESS;
}

