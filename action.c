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


struct event_lookup ev_lookup_table[] = {
	{ "onstart",        onstart},
	{ "onexit",         onexit},
	{ "onnewinput",     onnewinput},
	{ "button1",        button1},
	{ "button2",        button2},
	{ "button3",        button3},
	{ "button4",        button4},
	{ "button5",        button5},
	{ "button6",        button6},
	{ "button7",        button7},
	{ "entertitle",     entertitle},
	{ "leavetitle",     leavetitle},
	{ "enterslave",     enterslave},
	{ "leaveslave",     leaveslave},
	{ "sigusr1",        sigusr1},
	{ "sigusr2",        sigusr2},
	{ "keymarker",      keymarker},
	{ 0, 0 }
};

struct action_lookup  ac_lookup_table[] = {
	{ "print",          a_print },
	{ "exec",           a_exec},
	{ "exit",           a_exit},
	{ "collapse",       a_collapse},
	{ "uncollapse",     a_uncollapse},
	{ "togglecollapse",	a_togglecollapse},
	{ "stick",          a_stick},
	{ "unstick",        a_unstick},
	{ "togglestick",    a_togglestick},
	{ "hide",           a_hide},
	{ "unhide",         a_unhide},
	{ "togglehide",     a_togglehide},
	{ "scrollup",       a_scrollup},
	{ "scrolldown",     a_scrolldown},
	{ "menuprint",      a_menuprint},
	{ "menuprint_noparse",      a_menuprint_noparse},
	{ "menuexec",       a_menuexec},
	{ "raise",          a_raise},
	{ "lower",          a_lower},
	{ "scrollhome",     a_scrollhome},
	{ "scrollend",      a_scrollend},
	{ "grabkeys",       a_grabkeys},
	{ "ungrabkeys",     a_ungrabkeys},
	{ "grabmouse",       a_grabmouse},
	{ "ungrabmouse",     a_ungrabmouse},
	{ 0, 0 }
};

ev_list *head = NULL;

static int
new_event(long evid) {
	ev_list *item, *newitem;

	if(!head) {
		head = emalloc(sizeof (ev_list));
		head->id = evid;
		head->next = NULL;
	}
	else {
		item = head;
		/* check if we already handle this event */
		while(item) {
			if(item->id == evid)
				return 0;
			item = item->next;
		}
		item = head;
		while(item->next)
			item = item->next;

		newitem = emalloc(sizeof (ev_list));
		newitem->id = evid;
		item->next = newitem;
		newitem->next = NULL;
	}
	return 0;
}

static void
add_handler(long evid, int hpos, handlerf* hcb) {
	ev_list *item;

	item = head;
	while(item) {
		if(item->id == evid) {
			if(hpos < MAXACTIONS) {
				item->action[hpos] = emalloc(sizeof(As));
				item->action[hpos]->handler = hcb;
			}
			break;
		}
		item = item->next;
	}
}

static void
add_option(long evid, int hpos, int opos, char* opt) {
	ev_list *item;

	item = head;
	while(item) {
		if(item->id == evid) {
			if(opos < MAXOPTIONS) {
				item->action[hpos]->options[opos] = estrdup(opt);
				item->action[hpos]->options[opos+1] = NULL;
			}
			break;
		}
		item = item->next;
	}
}

int
find_event(long evid) {
	ev_list *item;

	item = head;
	while(item) {
		if(item->id == evid)
			return item->id;
		item = item->next;
	}

	return -1;
}

void
do_action(long evid) {
	int i;
	ev_list *item;

	item = head;
	while(item) {
		if(item->id == evid)
			break;
		item = item->next;
	}

	if(item) {
		for(i=0; item->action[i]->handler; i++) {
			item->action[i]->handler(item->action[i]->options);
		}
	}
}

int
get_ev_id(const char *evname) {
	int i;
	KeySym ks;

	/* check for keyboard event */
	if((!strncmp(evname, "key_", 4))
			&& ((ks = XStringToKeysym(evname+4)) != NoSymbol)) {
		return ks+keymarker;
	}

	/* own events */
	for(i=0; ev_lookup_table[i].name; i++) {
		if(strncmp(ev_lookup_table[i].name, evname, strlen(ev_lookup_table[i].name)) == 0)
			return ev_lookup_table[i].id;
	}
	return -1;
}

handlerf *
get_action_handler(const char *acname) {
	int i;

	for(i=0; ac_lookup_table[i].name; i++) {
		if(strcmp(ac_lookup_table[i].name, acname) == 0)
			return ac_lookup_table[i].handler;
	}
	return NULL;
}


void
free_event_list(void) {
	int i;
	ev_list *item;

	item = head;
	while(item) {
		for(i=0; item->action[i]->handler; i++)
			free(item->action[i]);
		item = item->next;
	}
}

void
fill_ev_table(char *input) {
	char *str1, *str2, *str3, *str4,
		 *token, *subtoken, *kommatoken, *dptoken;
	char *saveptr1=NULL,
		 *saveptr2=NULL,
		 *saveptr3=NULL,
		 *saveptr4=NULL;
	int j, i=0, k=0;
	long eid=0;
	handlerf *ah=0;

	for (j = 1, str1 = input; ; j++, str1 = NULL) {
		token = strtok_r(str1, ";", &saveptr1);
		if (token == NULL)
			break;

		for (str2 = token; ; str2 = NULL) {
			subtoken = strtok_r(str2, "=", &saveptr2);
			if (subtoken == NULL)
				break;
			if( (str2 == token) && ((eid = get_ev_id(subtoken)) != -1))
				;
			else if(eid == -1)
				break;

			for (str3 = subtoken; ; str3 = NULL) {
				kommatoken = strtok_r(str3, ",", &saveptr3);
				if (kommatoken == NULL)
					break;

				for (str4 = kommatoken; ; str4 = NULL) {
					dptoken = strtok_r(str4, ":", &saveptr4);
					if (dptoken == NULL) {
						break;
					}
					if(str4 == kommatoken && str4 != token && eid != -1) {
						if((ah = get_action_handler(dptoken)) != NULL) {
							new_event(eid);
							add_handler(eid, i, ah);
							i++;
						}
					}
					else if(str4 != token && eid != -1 && ah) {
						add_option(eid, i-1, k, dptoken);
						k++;
					}
					else if(!ah)
						break;
				}
				k=0;
			}
			new_event(eid);
			add_handler(eid, i, NULL);
			i=0;
		}
	}
}


/* actions */
int
a_exit(char * opt[]) {
	if(opt[0])
		dzen.ret_val = atoi(opt[0]);
	dzen.running = False;
	return 0;
}

int
a_collapse(char * opt[]){
	(void)opt;
	if(!dzen.slave_win.ishmenu
			&& dzen.slave_win.max_lines
			&& !dzen.slave_win.issticky) {
		XUnmapWindow(dzen.dpy, dzen.slave_win.win);
	}
	return 0;
}

int
a_uncollapse(char * opt[]){
	int i;
	(void)opt;
	if(!dzen.slave_win.ishmenu
			&& dzen.slave_win.max_lines
			&& !dzen.slave_win.issticky) {
		XMapRaised(dzen.dpy, dzen.slave_win.win);
		for(i=0; i < dzen.slave_win.max_lines; i++)
			XMapWindow(dzen.dpy, dzen.slave_win.line[i]);
	}
	return 0;
}

int
a_togglecollapse(char * opt[]){
	XWindowAttributes wa;
	(void)opt;

	if(dzen.slave_win.max_lines &&
			(XGetWindowAttributes(dzen.dpy, dzen.slave_win.win, &wa), wa.map_state == IsUnmapped))
		a_uncollapse(NULL);
	else
		a_collapse(NULL);

	return 0;
}

int
a_stick(char * opt[]) {
	(void)opt;
	if(!dzen.slave_win.ishmenu
			&& dzen.slave_win.max_lines)
		dzen.slave_win.issticky = True;
	return 0;
}

int
a_unstick(char * opt[]) {
	(void)opt;
	if(!dzen.slave_win.ishmenu
			&& dzen.slave_win.max_lines)
		dzen.slave_win.issticky = False;
	return 0;
}

int
a_togglestick(char * opt[]) {
	(void)opt;
	if(!dzen.slave_win.ishmenu
			&& dzen.slave_win.max_lines)
		dzen.slave_win.issticky = dzen.slave_win.issticky ? False : True;
	return 0;
}

static void
scroll(int n) {
	if(dzen.slave_win.tcnt <= dzen.slave_win.max_lines)
		return;
	if(dzen.slave_win.first_line_vis + n < 0) {
		dzen.slave_win.first_line_vis = 0;
		dzen.slave_win.last_line_vis = dzen.slave_win.max_lines;
	}
	else if(dzen.slave_win.last_line_vis + n > dzen.slave_win.tcnt) {
		dzen.slave_win.first_line_vis = dzen.slave_win.tcnt - dzen.slave_win.max_lines;
		dzen.slave_win.last_line_vis = dzen.slave_win.tcnt;
	}
	else {
		dzen.slave_win.first_line_vis += n;
		dzen.slave_win.last_line_vis  += n;
	}

	x_draw_body();
}

int
a_scrollup(char * opt[]) {
	int n=1;

	if(opt[0])
		n = atoi(opt[0]);
	if(dzen.slave_win.max_lines)
			scroll(-1*n);

	return 0;
}

int
a_scrolldown(char * opt[]) {
	int n=1;

	if(opt[0])
		n = atoi(opt[0]);
	if(dzen.slave_win.max_lines)
		scroll(n);

	return 0;
}

int
a_hide(char * opt[]) {
	int n=1;
	
	
	printf("n:%d\n", n);
	if(!dzen.title_win.ishidden) {
		if(!dzen.slave_win.ishmenu)
			XResizeWindow(dzen.dpy, dzen.title_win.win, dzen.title_win.width, 1);
		else
			XResizeWindow(dzen.dpy, dzen.slave_win.win, dzen.title_win.width, 1);

		dzen.title_win.ishidden = True;
	}
	return 0;
}

int
a_unhide(char * opt[]) {
	(void)opt;
	if(dzen.title_win.ishidden) {
		if(!dzen.slave_win.ishmenu)
			XResizeWindow(dzen.dpy, dzen.title_win.win, dzen.title_win.width, dzen.line_height);
		else
			XResizeWindow(dzen.dpy, dzen.slave_win.win, dzen.title_win.width, dzen.line_height);

		dzen.title_win.ishidden = False;
	}
	return 0;
}

int
a_togglehide(char * opt[]) {

	dzen.title_win.ishidden ?
		a_unhide(NULL) :
		a_hide(opt);

	return 0;
}

int
a_exec(char * opt[]) {
	int i;

	if(opt)
		for(i=0; opt[i]; i++)
			if(opt[i]) spawn(opt[i]);
	return 0;
}

int
a_print(char * opt[]) {
	int i;

	if(opt)
		for(i=0; opt[i]; i++)
			puts(opt[i]);
	fflush(stdout);
	return 0;
}

int
a_menuprint(char * opt[]) {
	char *text;
	int i;

	if(dzen.slave_win.ismenu && dzen.slave_win.sel_line != -1
			&& (dzen.slave_win.sel_line + dzen.slave_win.first_line_vis) < dzen.slave_win.tcnt) {
		text = parse_line(NULL, dzen.slave_win.sel_line, 0, 0, 1);
		printf("%s", text);
		if(opt)
			for(i=0; opt[i]; ++i)
				printf("%s", opt[i]);
		puts("");
		fflush(stdout);
		dzen.slave_win.sel_line = -1;
		free(text);
	}
	return 0;
}

int
a_menuprint_noparse(char * opt[]) {
	int i;

	if(dzen.slave_win.ismenu && dzen.slave_win.sel_line != -1
			&& (dzen.slave_win.sel_line + dzen.slave_win.first_line_vis) < dzen.slave_win.tcnt) {
		printf("%s", dzen.slave_win.tbuf[dzen.slave_win.sel_line]);
		if(opt)
			for(i=0; opt[i]; ++i)
				printf("%s", opt[i]);
		puts("");
		fflush(stdout);
		dzen.slave_win.sel_line = -1;
	}
	return 0;
}

int
a_menuexec(char * opt[]) {
	char *text;
	(void)opt;

	if(dzen.slave_win.ismenu && dzen.slave_win.sel_line != -1
			&& (dzen.slave_win.sel_line + dzen.slave_win.first_line_vis) < dzen.slave_win.tcnt) {
		text = parse_line(NULL, dzen.slave_win.sel_line, 0, 0, 1);
		spawn(text);
		dzen.slave_win.sel_line = -1;
		free(text);
	}
	return 0;
}

int
a_raise(char * opt[]) {
	(void)opt;
	XRaiseWindow(dzen.dpy, dzen.title_win.win);

	if(dzen.slave_win.max_lines)
		XRaiseWindow(dzen.dpy, dzen.slave_win.win);
	return 0;
}

int
a_lower(char * opt[]) {
	(void)opt;
	XLowerWindow(dzen.dpy, dzen.title_win.win);

	if(dzen.slave_win.max_lines)
		XLowerWindow(dzen.dpy, dzen.slave_win.win);
	return 0;
}

int
a_scrollhome(char * opt[]) {
	(void)opt;
	if(dzen.slave_win.max_lines) {
		dzen.slave_win.first_line_vis = 0;
		dzen.slave_win.last_line_vis  = dzen.slave_win.max_lines;

		x_draw_body();
	}
	return 0;
}

int
a_scrollend(char * opt[]) {
	(void)opt;
	if(dzen.slave_win.max_lines) {
		dzen.slave_win.first_line_vis = dzen.slave_win.tcnt - dzen.slave_win.max_lines ;
		dzen.slave_win.last_line_vis  = dzen.slave_win.tcnt;

		x_draw_body();
	}
	return 0;
}

int
a_grabkeys(char * opt[]) {
	(void)opt;
	XGrabKeyboard(dzen.dpy, RootWindow(dzen.dpy, dzen.screen),
			True, GrabModeAsync, GrabModeAsync, CurrentTime);
	return 0;
}

int
a_ungrabkeys(char * opt[]) {
	(void)opt;
	XUngrabKeyboard(dzen.dpy, CurrentTime);
	return 0;
}

int
a_grabmouse(char * opt[]) {
	(void)opt;
	XGrabPointer(dzen.dpy, RootWindow(dzen.dpy, dzen.screen),
			True, ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	return 0;
}

int
a_ungrabmouse(char * opt[]) {
	(void)opt;
	XUngrabPointer(dzen.dpy, CurrentTime);
	return 0;
}

