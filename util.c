#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "util.h"

char *smprintf(char *fmt, ...) {
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

char *execscript(char *cmd) {
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
  retval[strlen(retval) - 1] = '\0';

  return smprintf("%s", retval);
}

int runevery(time_t *ltime, int sec) {
  /* return 1 if sec elapsed since last run
   * else return 0
   */
  time_t now = time(NULL);
  if (difftime(now, *ltime) >= sec) {
    *ltime = now;
    return 1;
  } else
    return 0;
}