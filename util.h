#ifndef _DWMSTATUS_H
#define _DWMSTATUS_H

#include <time.h>

/* colors */
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define RST "\x1B[0m"

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
