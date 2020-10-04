/* parser function and structure where all found tokens are stored */

#ifndef PARSE_H__
#define PARSE_H__

#include "buffer.h"

typedef struct Tokens {
	char cmd, suffix, mark;
	int start, end, target;
	char *arg;
} Tokens;

int parse(char *exp, Tokens *tk, Buffer *buff);

#endif
