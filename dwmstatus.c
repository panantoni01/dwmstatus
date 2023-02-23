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


/* include this into your dwmstatus.c and use get_vol() as volume.
 * if your audio card and subunit numbers differ from 0,0 you might havo
 * to use amixer, aplay and the /proc/asound file tree to adapt.
 *
 * I had compilation issues. As result i had to drop the -std=c99 and
 * -pedantic flags from the config.mk
 */

#include <alsa/asoundlib.h>
#include <alsa/control.h>

int
get_vol(void) // volume
{
    int vol;
    snd_hctl_t *hctl;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_value_t *control;

// To find card and subdevice: /proc/asound/, aplay -L, amixer controls
    snd_hctl_open(&hctl, "hw:0", 0);
    snd_hctl_load(hctl);

    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);

// amixer controls
    snd_ctl_elem_id_set_name(id, "Master Playback Volume");

    snd_hctl_elem_t *elem = snd_hctl_find_elem(hctl, id);

    snd_ctl_elem_value_alloca(&control);
    snd_ctl_elem_value_set_id(control, id);

    snd_hctl_elem_read(elem, control);
    vol = (int)snd_ctl_elem_value_get_integer(control,0);

    snd_hctl_close(hctl);
    return vol;
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


char* cpustat() { // cpustat
	static struct cpustat st[2] = {
		{0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0}
	};
	static int curr = 0;
	int prev = 1 - curr;
	FILE *fp;
	char cpun[255];
	
	fp = fopen("/proc/stat", "r");
    fscanf(fp, "%s %lu %lu %lu %lu %lu %lu %lu", cpun, &(st[curr].t_user), 
		&(st[curr].t_nice), &(st[curr].t_system), &(st[curr].t_idle), &(st[curr].t_iowait), 
		&(st[curr].t_irq), &(st[curr].t_softirq));
	fclose(fp);

	int idle_prev = (st[prev].t_idle) + (st[prev].t_iowait);
    int idle_cur = (st[curr].t_idle) + (st[curr].t_iowait);

    int nidle_prev = (st[prev].t_user) + (st[prev].t_nice) + (st[prev].t_system) + (st[prev].t_irq) + (st[prev].t_softirq);
    int nidle_cur = (st[curr].t_user) + (st[curr].t_nice) + (st[curr].t_system) + (st[curr].t_irq) + (st[curr].t_softirq);

    int total_prev = idle_prev + nidle_prev;
    int total_cur = idle_cur + nidle_cur;

    double totald = (double) total_cur - (double) total_prev;
    double idled = (double) idle_cur - (double) idle_prev;

    long cpu_perc = (long)((1000 * (totald - idled) / totald + 1) / 10);

	curr = 1 - curr;
	if (cpu_perc > 80)
		return(smprintf(RED"%ld%%"RST, cpu_perc));
	if (cpu_perc > 60)
		return(smprintf(YEL"%ld%%"RST, cpu_perc));
	return(smprintf("%ld%%", cpu_perc));
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

