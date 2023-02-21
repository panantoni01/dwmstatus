/*
 * Copy me if you can.
 * by 20h
 */

#define _BSD_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <X11/Xlib.h>

#include "dwmstatus.h"

char *tzwarsaw = "Europe/Warsaw";

static Display *dpy;

char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

void
settz(char *tzname)
{
	setenv("TZ", tzname, 1);
}

char *
mktimes(char *fmt, char *tzname)
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

void
setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}


char *
readfile(char *base, char *file)
{
	char *path, line[513];
	FILE *fd;

	memset(line, 0, sizeof(line));

	path = smprintf("%s/%s", base, file);
	fd = fopen(path, "r");
	free(path);
	if (fd == NULL)
		return NULL;

	if (fgets(line, sizeof(line)-1, fd) == NULL) {
		fclose(fd);
		return NULL;
	}
	fclose(fd);

	return smprintf("%s", line);
}

char *
execscript(char *cmd)
{
	FILE *fp;
	char retval[1025], *rv;

	memset(retval, 0, sizeof(retval));

	fp = popen(cmd, "r");
	if (fp == NULL)
		return smprintf("");

	rv = fgets(retval, sizeof(retval), fp);
	pclose(fp);
	if (rv == NULL)
		return smprintf("");
	retval[strlen(retval)-1] = '\0';

	return smprintf("%s", retval);
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
get_vol(void)
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

char *get_freespace(char *mntpt){
    struct statvfs data;
    double total, used = 0;

    if ( (statvfs(mntpt, &data)) < 0){
		fprintf(stderr, "can't get info on disk.\n");
		return("?");
    }
    total = (data.f_blocks * data.f_frsize);
    used = (data.f_blocks - data.f_bfree) * data.f_frsize ;
    return(smprintf("%.0f", (used/total*100)));
}


void get_cpustat(struct cpustat* st) {
	FILE *fp;
	char cpun[255];
	
	fp = fopen("/proc/stat", "r");
    fscanf(fp, "%s %lu %lu %lu %lu %lu %lu %lu", cpun, &(st->t_user), &(st->t_nice),
        &(st->t_system), &(st->t_idle), &(st->t_iowait), &(st->t_irq), &(st->t_softirq));
	fclose(fp);
}

static inline void cpustat_init(struct cpustat** st) {
    *st = (struct cpustat *) malloc(sizeof(struct cpustat));
    if (*st == NULL) {
		perror("malloc");
		exit(1);
	}
	get_cpustat(*st);
}

long calculate_load(struct cpustat *prev, struct cpustat *cur)
{
    int idle_prev = (prev->t_idle) + (prev->t_iowait);
    int idle_cur = (cur->t_idle) + (cur->t_iowait);

    int nidle_prev = (prev->t_user) + (prev->t_nice) + (prev->t_system) + (prev->t_irq) + (prev->t_softirq);
    int nidle_cur = (cur->t_user) + (cur->t_nice) + (cur->t_system) + (cur->t_irq) + (cur->t_softirq);

    int total_prev = idle_prev + nidle_prev;
    int total_cur = idle_cur + nidle_cur;

    double totald = (double) total_cur - (double) total_prev;
    double idled = (double) idle_cur - (double) idle_prev;

    double cpu_perc = (1000 * (totald - idled) / totald + 1) / 10;

    return (long)cpu_perc;
}

void update_cpustat(struct cpustat **prev, struct cpustat **cur) {
	free(*prev);
	*prev = *cur;
	
	cpustat_init(cur);
	get_cpustat(*cur);
}

int
main(void)
{
	long time = 0;
	char *status;
	char *tmwar;
	char *kbmap;
	int vol;
	char* du;
	struct cpustat *cpu_prev;
	struct cpustat *cpu_cur;
	long load;
	

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	cpustat_init(&cpu_prev);
	cpustat_init(&cpu_cur);

	for (;;sleep(INTERVAL), time++) {
		if (time % KBMAP_REFRESH == 0)
			kbmap = execscript("setxkbmap -query | grep layout | cut -d':' -f 2- | tr -d ' '");
		if (time % DU_REFRESH == 0)
			du    = get_freespace("/");
		tmwar = mktimes("%a %d %b %Y %H:%M:%S ", tzwarsaw);
		vol   = get_vol();
		update_cpustat(&cpu_prev, &cpu_cur);
		load = calculate_load(cpu_prev, cpu_cur);

		status = smprintf(" \uF11C %s | \uF2DB %ld%% | \uF0A0 %s%% | \uF027 %d%% | \uF017 %s",
				kbmap, load, du, vol, tmwar);
		setstatus(status);

		if (time % KBMAP_REFRESH == 0)
			free(kbmap);
		if (time % DU_REFRESH == 0)
			free(du);
		free(tmwar);
		free(status);
	}

	free(cpu_prev);
	free(cpu_cur);

	XCloseDisplay(dpy);

	return 0;
}

