/* general functions for use in all translation units */

#ifndef UTIL_H__
#define UTIL_H__

extern int verbose;

void *emalloc(size_t size);
void *ecalloc(size_t nmemb, size_t size);
void *erealloc(void *ptr, size_t size);
char *estrdup(char *str);
char *readcmd(const char *prompt);
char *readline(FILE *stream);
void error(const char *msg);

#endif
