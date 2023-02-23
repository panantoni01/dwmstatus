/*
 * Copy me if you can.
 * by 20h
 */

#define _BSD_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/Xlib.h>

#include "util.h"

char* cpustat();
int   get_vol();
char* mktimes(char*, char*);
char* get_freespace(char*);
char* getmem();

char *tzwarsaw = "Europe/Warsaw";

static Display *dpy;

static void
setstatus(char *str) 
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}


int
main(void)
{
	char *status = NULL;
	char *tmwar = NULL;
	char *kbmap = NULL;
	int vol = 0;
	char* du = NULL;
	char* cpu_perc = NULL;
	char* memory = NULL;
	time_t sec10 = 0;
	

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(1)) {
		if (runevery(&sec10, 10)) {
			free(kbmap);
			free(du);
			kbmap = execscript("setxkbmap -query | grep layout | cut -d':' -f 2- | tr -d ' '");
			du    = get_freespace("/");
		}
		
		cpu_perc = cpustat();
		memory   = getmem();
		vol      = get_vol();
		tmwar    = mktimes("%a %d %b %Y %H:%M:%S ", tzwarsaw);

		status = smprintf(" \uF11C %s | \uF2DB %s | \uF538 %s | \uF0A0 %s | \uF027 %d%% | \uF017 %s",
				kbmap, cpu_perc, memory, du, vol, tmwar);
		setstatus(status);

		free(tmwar);
		free(status);
		free(cpu_perc);
		free(memory);
	}

	XCloseDisplay(dpy);

	return 0;
}

