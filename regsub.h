#ifndef _REGSUB_H
#define REGSUB_H 1

#include <regex.h>
#define REG_GLOBAL 1
#define REG_EXIT_ESCAPE -1
#define REG_EXIT_CAPTURE -2
#define REG_EXIT_REALLOC -3

int regsub(const regex_t *preg, char *str, char *rep, int flags);

#endif
