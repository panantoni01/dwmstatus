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
int get_vol();

char *tzwarsaw = "Europe/Warsaw";

static Display *dpy;

static void
setstatus(char *str) 
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

void
settz(char *tzname) // time
{
	setenv("TZ", tzname, 1);
}

char *
mktimes(char *fmt, char *tzname) // time
{
	char buf[129];
	time_t tim;
	struct tm *timtm;

	settz(tzname);
	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL)
		return smprintf("");

	if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		return smprintf("");
	}

	return smprintf("%s", buf);
}


#include <sys/statvfs.h>

char *get_freespace(char *mntpt){ // du
    struct statvfs data;
    double total, used = 0;
	double result;

    if ( (statvfs(mntpt, &data)) < 0){
		fprintf(stderr, "can't get info on disk.\n");
		return("?");
    }
    total = (data.f_blocks * data.f_frsize);
    used = (data.f_blocks - data.f_bfree) * data.f_frsize ;
    result = (used/total*100);

	if (result > 80)
		return(smprintf(RED"%.0f%%"RST, result));
	if (result > 60)
		return(smprintf(YEL"%.0f%%"RST, result));
	return(smprintf("%.0f%%", result));
}


char* getmem() { // memory
    FILE *fp;
    long total, free, available, result;

    fp = fopen("/proc/meminfo", "r");
    fscanf(fp, "MemTotal: %ld kB\n", &total);
    fscanf(fp, "MemFree: %ld kB\n", &free);
    fscanf(fp, "MemAvailable: %ld kB\n", &available);
    fclose(fp);
	result = (100 * (total - available) / total);

	if (result > 80)
		return(smprintf(RED"%ld%%"RST, result));
	if (result > 60)
		return(smprintf(YEL"%ld%%"RST, result));
	return(smprintf("%ld%%", result));
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

