#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "util.h"

static void print_literal(char *str);

size_t
buff_load(Buffer *buff, const char *fp)
{
	FILE *stream = fopen(fp, "r");
	if (!stream) {
		perror(fp);
		return 0;
	}
	char *line;
	while ((line = readline(stream))) {
		buff_ins(buff, buff->tail, line);
		free(line);
	}
	buff->modified = 0;
	size_t size = ftell(stream);
	fclose(stream);
	return size;
}

size_t
buff_save(Buffer *buff, const char *fp)
{
	FILE *stream = fopen(fp, "w");
	if (!stream) {
		perror(fp);
		return 0;
	}
	for (Line *line = buff->head; line; line = line->next)
		fprintf(stream, "%s\n", line->data);
	size_t size = ftell(stream);
	fclose(stream);
	buff->modified = 0;
	return size;
}

Line *
buff_get(Buffer *buff, int pos)
{
	Line *line;
	int i;

	if (pos == 0)
		return NULL;

	if (pos < buff->last_line / 2) {
		i = 1;
		line = buff->head;
		while (line != buff->tail && i < pos) {
			line = line->next;
			i++;
		}
	} else {
		i = buff->last_line;
		line = buff->tail;
		while (line != buff->head && i > pos) {
			line = line->prev;
			i--;
		}
	}
	return line;
}

Line *
buff_ins(Buffer *buff, Line *target, char *data)
{
	Line *line;

	line = emalloc(sizeof(Line));
	line->next = line->prev = NULL;
	line->data = estrdup(data);
	buff->last_line++;

	if (!buff->head) {
		line->next = line->prev = NULL;
		buff->head = buff->tail = line;
		return line;
	}

	return buff_move(buff, target, line);
}

Line *
buff_rm(Buffer *buff, Line *line) {
	Line *next;

	buff->modified = 1;
	buff->last_line--;

	if (line->prev) line->prev->next = line->next;
	if (line->next) line->next->prev = line->prev;
	if (line == buff->head) buff->head = line->next;
	if (line == buff->tail) buff->tail = line->prev;
	next = line->next;
	free(line->data);
	free(line);
	return next ? next : buff->tail;
}

void
print_literal(char *str)
{
	for (int i = 0;;i++) {
		if (i > 80) {
			printf("\\\n");
			continue;
		}
		switch (*str) {
		case '\\':
			printf("\\\\");
			break;
		case '\a':
			printf("\\a");
			break;
		case '\b':
			printf("\\b");
			break;
		case '\f':
			printf("\\f");
			break;
		case '\r':
			printf("\\r");
			break;
		case '\t':
			printf("\\t");
			break;
		case '\v':
			printf("\\v");
			break;
		case '\0':
			printf("$\n");
			return;
		case '$':
			printf("\\$");
			return;
		default:
			putchar(*str);
		}
		str++;
	}
}

void
buff_print(Buffer *buff, int start, int end, int num, int literal)
{
	Line *line;
	int i;

	line = buff_get(buff, start);
	for (i = start; line && i <= end; line = line->next, i++) {
		if (num)
			printf("%-8d ", i);

		if (literal)
			print_literal(line->data);
		else
			printf("%s\n", line->data);
	}
}

Line *
buff_move(Buffer *buff, Line *target, Line *line)
{
	buff->modified = 1;

	if (line == buff->head) buff->head = line->next;
	if (line->prev) line->prev->next = line->next;
	if (line->next) line->next->prev = line->prev;

	if (!target) {
		if (buff->head) buff->head->prev = line;
		line->next = buff->head;
		line->prev = NULL;
		buff->head = line;
		return line;
	}

	if (target == buff->tail) buff->tail = line;
	if (target->next) target->next->prev = line;
	line->prev = target;
	line->next = target->next;
	target->next = line;
	return line;
}

/* to test if all nodes are properly linked */
void
buff_print_rev(Buffer *buff, int start, int end, int num, int literal)
{
	Line *line;
	int i;

	line = buff_get(buff, end);
	for (i = end; line && i >= start; line = line->prev, i--) {
		if (num)
			printf("%-8d ", i);

		if (literal)
			print_literal(line->data);
		else
			printf("%s\n", line->data);
	}
}

void
buff_free(Buffer *buff)
{
	Line *tmp;
	Line *line;

	for (line = buff->head; line != NULL; line = tmp) {
		tmp = line->next;
		free(line->data);
		free(line);
	}
	buff->head = buff->tail = NULL;
	buff->last_line = 0;
}
