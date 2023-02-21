#ifndef _DWMSTATUS_H
#define _DWMSTATUS_H

#define INTERVAL 1
#define KBMAP_REFRESH 10 /* refresh kbmap every 10 INTERVALS*/
#define DU_REFRESH 30    /* refresh du every 30 INTERVALS*/


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

