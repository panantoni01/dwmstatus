#ifndef _DWMSTATUS_H
#define _DWMSTATUS_H

struct cpustat {
    unsigned long t_user;
    unsigned long t_nice;
    unsigned long t_system;
    unsigned long t_idle;
    unsigned long t_iowait;
    unsigned long t_irq;
    unsigned long t_softirq;
};


#endif /* _DWMSTATUS_H */

