#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "util.h"

char *
readfile(char *base, char *file)
{
	char *path, line[513];
	FILE *fd;

	memset(line, 0, sizeof(line));

	path = smprintf("%s/%s", base, file);
	fd = fopen(path, "r");
	if (fd == NULL) {
		perror("fopen");
		exit(1);
	}
	free(path);

	if (fgets(line, sizeof(line)-1, fd) == NULL) {
		perror("fgets");
		exit(1);
	}
	fclose(fd);

	return smprintf("%s", line);
}

char *
getbattery(char *base)
{
	char *co;
	int descap, remcap;
	float percentage;

	descap = -1;
	remcap = -1;

	co = readfile(base, "present");
	if (co[0] != '1') {
		free(co);
		return smprintf("not present");
	}
	free(co);

	co = readfile(base, "energy_full");
	sscanf(co, "%d", &descap);
	free(co);

	co = readfile(base, "energy_now");
	sscanf(co, "%d", &remcap);
	free(co);

	if (remcap < 0 || descap < 0)
		return smprintf("invalid");

	
	percentage = ((float)remcap / (float)descap) * 100;
  if (percentage < 10)
    return smprintf(COL_URG "%.0f%%" COL_NORM, percentage);
	if (percentage < 30)
		return smprintf(COL_WARN "%.0f%%" COL_NORM, percentage);
	return smprintf("%.0f%%", percentage);
}

