/* buffer structure that represents the currently edited file */

#ifndef BUFFER_H__
#define BUFFER_H__

typedef struct Line Line;

struct Line {
	Line *prev;
	Line *next;
	char *data;
};

typedef struct {
	Line *head;
	Line *tail;
	int last_line, curr_line;
	int marks[256];
	int modified;
} Buffer;

size_t buff_load(Buffer *buff, const char *fp);
size_t buff_save(Buffer *buff, const char *fp);
Line *buff_get(Buffer *buff, int pos);
Line *buff_ins(Buffer *buff, Line *target, char *data);
Line *buff_rm(Buffer *buff, Line *target);
Line *buff_move(Buffer *buff, Line *target, Line *line);
void buff_print(Buffer *buff, int start, int end, int num, int literal);
void buff_print_rev(Buffer *buff, int start, int end, int num, int literal);
void buff_free(Buffer *buff);

#endif
