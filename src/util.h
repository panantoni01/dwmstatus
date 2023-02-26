#ifndef _DWMSTATUS_H
#define _DWMSTATUS_H

#include <time.h>

/* colors */
#define COL_NORM "\x01"
#define COL_SEL  "\x02"
#define COL_WARN "\x03"
#define COL_URG  "\x04"

struct cpustat {
  unsigned long t_user;
  unsigned long t_nice;
  unsigned long t_system;
  unsigned long t_idle;
  unsigned long t_iowait;
  unsigned long t_irq;
  unsigned long t_softirq;
};

char *smprintf(char *, ...);
char *execscript(char *);
int runevery(time_t *, int);

#endif /* _DWMSTATUS_H */
