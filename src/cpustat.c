#include <stdio.h>

#include "util.h"

static long calculate_perc(struct cpustat *prev, struct cpustat *curr) {
  int idle_prev = (prev->t_idle) + (prev->t_iowait);
  int idle_cur = (curr->t_idle) + (curr->t_iowait);

  int nidle_prev = (prev->t_user) + (prev->t_nice) + (prev->t_system) +
                   (prev->t_irq) + (prev->t_softirq);
  int nidle_cur = (curr->t_user) + (curr->t_nice) + (curr->t_system) +
                  (curr->t_irq) + (curr->t_softirq);

  int total_prev = idle_prev + nidle_prev;
  int total_cur = idle_cur + nidle_cur;

  double totald = (double)total_cur - (double)total_prev;
  double idled = (double)idle_cur - (double)idle_prev;

  return (long)((1000 * (totald - idled) / totald + 1) / 10);
}

char *cpustat() {
  static struct cpustat st[2] = {{0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}};
  static int curr = 0;
  int prev = 1 - curr;
  FILE *fp;
  char cpun[255];
  long cpu_perc;

  fp = fopen("/proc/stat", "r");
  fscanf(fp, "%s %lu %lu %lu %lu %lu %lu %lu", cpun, &(st[curr].t_user),
         &(st[curr].t_nice), &(st[curr].t_system), &(st[curr].t_idle),
         &(st[curr].t_iowait), &(st[curr].t_irq), &(st[curr].t_softirq));
  fclose(fp);

  cpu_perc = calculate_perc(&st[prev], &st[curr]);

  curr = 1 - curr;
  if (cpu_perc > 80)
    return (smprintf(COL_URG "%ld%%" COL_NORM, cpu_perc));
  if (cpu_perc > 60)
    return (smprintf(COL_WARN "%ld%%" COL_NORM, cpu_perc));
  return (smprintf("%ld%%", cpu_perc));
}
