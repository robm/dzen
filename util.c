/* 
 * (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * (C)opyright MMVII Robert Manea <rob dot manea  at gmail dot com>
 * See LICENSE file for license details.
 *
 */

#include "dzen.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ONEMASK ((size_t)(-1) / 0xFF)

void *
emalloc(unsigned int size) {
	void *res = malloc(size);

	if(!res)
		eprint("fatal: could not malloc() %u bytes\n", size);
	return res;
}

void
eprint(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

char *
estrdup(const char *str) {
	void *res = strdup(str);

	if(!res)
		eprint("fatal: could not malloc() %u bytes\n", strlen(str));
	return res;
}
void
spawn(const char *arg) {
	static const char *shell = NULL;

	if(!shell && !(shell = getenv("SHELL")))
		shell = "/bin/sh";
	if(!arg)
		return;
	/* The double-fork construct avoids zombie processes and keeps the code
	* clean from stupid signal handlers. */
	if(fork() == 0) {
		if(fork() == 0) {
			setsid();
			execl(shell, shell, "-c", arg, (char *)NULL);
			fprintf(stderr, "dzen: execl '%s -c %s'", shell, arg);
			perror(" failed");
		}
		exit(0);
	}
	wait(0);
}

