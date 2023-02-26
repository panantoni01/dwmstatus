#include <stdio.h>

#include "util.h"

char *getmem() {
  FILE *fp;
  long total, free, available, result;

  fp = fopen("/proc/meminfo", "r");
  fscanf(fp, "MemTotal: %ld kB\n", &total);
  fscanf(fp, "MemFree: %ld kB\n", &free);
  fscanf(fp, "MemAvailable: %ld kB\n", &available);
  fclose(fp);
  result = (100 * (total - available) / total);

  if (result > 80)
    return (smprintf(COL_URG "%ld%%" COL_NORM, result));
  if (result > 60)
    return (smprintf(COL_WARN "%ld%%" COL_NORM, result));
  return (smprintf("%ld%%", result));
}
