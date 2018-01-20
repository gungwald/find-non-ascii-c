#include <stdio.h>	/* perror */
#include <string.h>
#include <stdarg.h>	/* va_arg, etc */
#include <malloc.h>
#include <ctype.h>	/* tolower */

#include "java-string-compat.h"

/**
 * The caller must free the result, or else.
 */
char *concat(const char *s, const char *t)
{   
    size_t len; 
    char *r;    /* The result :-) */
    
    len = strlen(s) + strlen(t);
    r = (char *) malloc(sizeof(char) * (len + 1));
    if (r == NULL) {
        perror("malloc");
	return NULL;
    }
    strcpy(r, s);
    strcat(r, t);
    r[len] = '\0';
    return r;
}

/**
 * The caller must free the result, or else.
 */
char *concatv(const char *s, ...)
{
    va_list args;
    char *t;
    size_t len;
    char *r;

    /* Calculate the total length. */
    len = strlen(s);
    va_start(args, s);
    while ((t = va_arg(args, char *)) != NULL) {
        len += strlen(t);
    }
    va_end(args);

    /* Allocate memory for the result. */
    r = (char *) malloc((len + 1) * sizeof(char));
    if (r == NULL) {
        perror("malloc");
	return r;
    }

    /* Build the result string. */
    strcpy(r, s);
    va_start(args, s);
    while ((t = va_arg(args, char *)) != NULL) {
        strcat(r, t);
    }
    va_end(args);
    r[len] = '\0'; /* Guarantee the string terminator. */
    return r;
}

bool endsWith(const char *s, const char *ending)
{
	const char *sPtr = s + strlen(s) - strlen(ending);
	if (sPtr > s && strcmp(sPtr, ending) == 0) {
		return true;
	}
	else {
		return false;
	}
}

/**
 * Result must be freed by caller, if it is not NULL.
 */
char *toLowerCase(const char *s)
{
	char *lower = NULL;
	if (s != NULL) {
		lower = strdup(s);
		if (lower != NULL) {
			for (char *p = lower; *p; p++) {
				*p = tolower(*p);
			}
		}
	}
	return lower;
}

