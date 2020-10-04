/* definition of commands and the glorius main */

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arg.h"
#include "buffer.h"
#include "parser.h"
#include "util.h"

typedef struct {
	Buffer buff;
	Tokens tk;
	char *filename;
	char *prompt_str;
	int edit_attempts;
	int exit_attempts;
	int prompt;
	int running;
	int silent;
} State;

static void parse_opt(int argc, char *argv[], State *st);

static void Edit_cmd(State *st);
static void Quit_cmd(State *st);
static void change_cmd(State *st);
static void copy_cmd(State *st);
static void delete_cmd(State *st);
static void edit_cmd(State *st);
static void edit_helper(State *st);
static void filename_cmd(State *st);
static void help_cmd(State *st);
static void help_mode_cmd(State *st);
static void insert_cmd(State *st);
static void list_cmd(State *st);
static void mark_cmd(State *st);
static void move_cmd(State *st);
static void number_cmd(State *st);
static void print_cmd(State *st);
static void prompt_cmd(State *st);
static void quit_cmd(State *st);
static void write_cmd(State *st);

static void (*handler[256])(State *st) = {
	['E'] = Edit_cmd,
	['H'] = help_mode_cmd,
	['P'] = prompt_cmd,
	['Q'] = Quit_cmd,
	['a'] = insert_cmd,
	['c'] = change_cmd,
	['d'] = delete_cmd,
	['e'] = edit_cmd,
	['f'] = filename_cmd,
	['h'] = help_cmd,
	['i'] = insert_cmd,
	['k'] = mark_cmd,
	['l'] = list_cmd,
	['m'] = move_cmd,
	['n'] = number_cmd,
	['p'] = print_cmd,
	['q'] = quit_cmd,
	['t'] = copy_cmd,
	['w'] = write_cmd,
};

int verbose = 1;

void
parse_opt(int argc, char *argv[], State *st)
{
	ARGBEGIN {
	case 'p': 
		st->prompt_str = EARGF;
		st->prompt = 1;
		break;
	case 's':
		st->silent = 1;
		break;
	} ARGELSE {
		if (!st->filename) st->filename = strdup(*argv);
	} ARGEND
	if (argc && !st->filename) st->filename = strdup(*argv);
}

void
edit_helper(State *st)
{
	buff_free(&st->buff);
	size_t size = buff_load(&st->buff, st->filename);
	if (!size)
		error("Cannot open input file");
	else if (!st->silent)
		printf("%ld\n", size);
	st->buff.curr_line = st->buff.last_line;
}

void
edit_cmd(State *st)
{
	if (st->buff.modified && !st->edit_attempts) {
		error("Warning: buffer modified");
		st->edit_attempts = 1;
		return;
	}
	if (st->tk.arg) {
		free(st->filename);
		st->filename = estrdup(st->tk.arg);
	}
	if (st->filename)
		edit_helper(st);
	else
		error("No current filename");
}

void
Edit_cmd(State *st)
{
	if (st->tk.arg) {
		free(st->filename);
		st->filename = estrdup(st->tk.arg);
	}
	if (st->filename)
		edit_helper(st);
	else
		error("No current filename");
}

void
insert_cmd(State *st)
{
	Line *line;
	int line_nr;
	char *data;
	int i;

	line_nr = st->tk.end - (st->tk.cmd != 'a');
	line = buff_get(&st->buff, line_nr);
	for (i = line_nr; (data = readline(stdin)) && strcmp(data, "."); i++)
		line = buff_ins(&st->buff, line, data);
	st->tk.end = st->tk.start = i;
}

void
quit_cmd(State *st)
{
	if (st->buff.modified && !st->exit_attempts) {
		error("Warning: buffer modified");
		st->exit_attempts = 1;
		return;
	}
	st->running = 0;
}

void
Quit_cmd(State *st)
{
	st->running = 0;
}

void
print_cmd(State *st)
{
	int numbers = st->tk.suffix == 'n';
	int literals = st->tk.suffix == 'l';
	buff_print(&st->buff, st->tk.start, st->tk.end, numbers, literals);
}

void
number_cmd(State *st)
{
	int literals = st->tk.suffix == 'l';
	buff_print(&st->buff, st->tk.start, st->tk.end, 1, literals);
}

void
list_cmd(State *st)
{
	int numbers = st->tk.suffix == 'n';
	buff_print(&st->buff, st->tk.start, st->tk.end, numbers, 1);
}

void
help_mode_cmd(State *st)
{
	(void)st;
	verbose ^= 1;
}

void
help_cmd(State *st)
{
	(void)st;
	error(NULL);
}

void
prompt_cmd(State *st)
{
	st->prompt ^= 1;
}

void
write_cmd(State *st)
{
	if (!st->filename && st->tk.arg)
		st->filename = estrdup(st->tk.arg);
	else if (st->tk.arg) {
		buff_save(&st->buff, st->tk.arg);
		return;
	}
	if (st->filename)
		buff_save(&st->buff, st->filename);
	else
		error("No current filename");
}

void
filename_cmd(State *st)
{

	if (st->tk.arg) {
		free(st->filename);
		st->filename = estrdup(st->tk.arg);
	}
	if (st->filename)
		puts(st->filename);
	else
		error("No current filename");
}

void
delete_cmd(State *st)
{
	Line *line;

	line = buff_get(&st->buff, st->tk.start);
	for (int i = st->tk.start; i <= st->tk.end; i++)
		line = buff_rm(&st->buff, line);
	st->tk.end = st->tk.start;
}

void
mark_cmd(State *st)
{
	if (isascii(st->tk.mark))
		st->buff.marks[(int)st->tk.mark] = st->tk.end;
	else
		error("Invalid marker character");
}

void
change_cmd(State *st)
{
	delete_cmd(st);
	insert_cmd(st);
}

void
copy_cmd(State *st)
{
	Line *tline = NULL;
	Line *cline;
	Buffer buff = {0};
	int i;

	cline = buff_get(&st->buff, st->tk.start);
	for (i = 0; i <= st->tk.end - st->tk.start && cline; i++) {
		tline = buff_ins(&buff, tline, cline->data);
		cline = cline->next;
	}

	tline = buff_get(&st->buff, st->tk.target);
	for (cline = buff.head; cline; cline = cline->next)
		tline = buff_ins(&st->buff, tline, cline->data);
	buff_free(&buff);

	st->tk.end = st->tk.start = st->tk.target + i;
}

void
move_cmd(State *st)
{
	Line *line;
	Line *target;
	Line *tmp;
	int i;

	line = buff_get(&st->buff, st->tk.start);
	target = buff_get(&st->buff, st->tk.target);
	for (i = 0; i <= st->tk.end - st->tk.start; i++) {
		tmp = line->next;
		target = buff_move(&st->buff, target, line);
		line = tmp;
	}
	st->tk.end = st->tk.start = st->tk.target ? st->tk.target : 1;
}

int
main(int argc, char *argv[])
{
	char *cmd;
	State st = { .running = 1, .prompt_str = "*"};

 	parse_opt(argc, argv, &st);
 	if (st.filename) edit_helper(&st);

 	while (st.running) {
		if (st.prompt) printf("%s", st.prompt_str);
		if (!(cmd = readline(stdin))) break;
		
		if (*cmd == '\0' || !parse(cmd, &st.tk, &st.buff)) {
			free(cmd);
			continue;
		}
		free(cmd);

		if (isascii(st.tk.cmd) && handler[(int)st.tk.cmd])
			handler[(int)st.tk.cmd](&st);
		else if (!st.tk.cmd)
			buff_print(&st.buff, st.tk.start, st.tk.end, 0, 0);
		else {
			error("Unknown command");
			continue;
		}

		if (st.tk.cmd != 'q') st.exit_attempts = 0;
		if (st.tk.cmd != 'e') st.edit_attempts = 0;

		if (st.tk.suffix && !strchr("pnl", st.tk.cmd))
			print_cmd(&st);

		st.buff.curr_line = st.buff.last_line ? st.tk.end : 0;
 	}
	buff_free(&st.buff);
	free(st.filename);
}
