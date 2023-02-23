#include <stdio.h>

#include "util.h"

char*
getmem()
{
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