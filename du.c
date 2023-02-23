#include <sys/statvfs.h>
#include <stdio.h>

#include "util.h"

char *
get_freespace(char *mntpt)
{
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