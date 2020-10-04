#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define ALLOC(fn, ...) \
	void *p; \
	if (!(p = fn(__VA_ARGS__))) { \
		perror(#fn); \
		exit(1); \
	} \
	return p;

void *
emalloc(size_t size)
{
	ALLOC(malloc, size);
}

void *
ecalloc(size_t nmemb, size_t size)
{
	ALLOC(calloc, nmemb, size);
}

void *
erealloc(void *ptr, size_t size)
{
	void *p;
	if (!(p = realloc(ptr, size))) {
		free(ptr);
		perror("realloc");
		exit(1);
	}
	return p;
}

char *
estrdup(char *ptr)
{
	void *p;
	if (!(p = strdup(ptr))) {
		free(ptr);
		perror("strdup");
		exit(1);
	}
	return p;
}

void
error(const char *msg) 
{
	static const char *error_msg;
	if (msg) {
		puts("?");
		error_msg = msg;
		if (verbose) puts(msg);
	} else if (error_msg)
		puts(error_msg);
}

char *
readline(FILE *stream) {
	const int mem = 128;
	size_t len = 0;
	char *str = NULL;
	char *next_read;

	for (;;) {
		str = erealloc(str, len + mem);
		next_read = str + len;
		if (!(fgets(next_read, mem, stream))) {
			free(str);
			return NULL;
		}
		len = strlen(str);
		if (str[len - 1] == '\n') {
			str[strlen(str) - 1] = '\0';
			break;
		}
	}
	return erealloc(str, len);
}
