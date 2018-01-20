#ifndef JAVA_STRING_COMPAT_H
#define JAVA_STRING_COMPAT_H

#include <stdbool.h>	/* bool */

extern char *concat(const char *s, const char *t);
extern char *concatv(const char *s, ...);
extern bool endsWith(const char *s, const char *ending);
extern char *toLowerCase(const char *s);

#endif
