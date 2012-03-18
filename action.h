/*  
 * (C)opyright 2007-2009 Robert Manea <rob dot manea at gmail dot com>
 * See LICENSE file for license details.
 *
 */

#define MAXACTIONS 64
#define MAXOPTIONS 64

/* Event, Action data structures */
typedef struct AS As;
typedef struct _ev_list ev_list;
typedef int handlerf(char **);

enum ev_id {
	/* startup, exit, input */
	onstart, onexit, onnewinput,
	/* mouse buttons */
	button1, button2, button3, button4, button5,  button6,  button7,
	/* entering/leaving windows */
	entertitle, leavetitle, enterslave, leaveslave, 
	/* external signals */
	sigusr1, sigusr2,
	/* key event marker 
	 * must always be the last entry
	 */
	keymarker
};

struct _ev_list {
	long id;
	As *action[MAXACTIONS];
	ev_list *next;
};

struct event_lookup {
	const char *name;
	long id;
};

struct action_lookup {
	const char *name;
	int (*handler)(char **);
};

struct AS {
	char *options[MAXOPTIONS];
	int (*handler)(char **);
};


/* utility functions */
void do_action(long);
int get_ev_id(const char *);
handlerf *get_action_handler(const char *);
void fill_ev_table(char *);
void free_event_list(void);
int find_event(long);

/* action handlers */
int a_print(char **);
int a_exit(char **);
int a_exec(char **);
int a_collapse(char **);
int a_uncollapse(char **);
int a_togglecollapse(char **);
int a_stick(char **);
int a_unstick(char **);
int a_togglestick(char **);
int a_scrollup(char **);
int a_scrolldown(char **);
int a_hide(char **);
int a_unhide(char **);
int a_togglehide(char **);
int a_menuprint(char **);
int a_menuprint_noparse(char **);
int a_menuexec(char **);
int a_raise(char **);
int a_lower(char **);
int a_scrollhome(char **);
int a_scrollend(char **);
int a_grabkeys(char **);
int a_ungrabkeys(char **);
int a_grabmouse(char **);
int a_ungrabmouse(char **);

