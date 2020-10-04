#include <ctype.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "parser.h"
#include "util.h"

static int parse_range(char **exp, Tokens *tk, Buffer *buff);
static int parse_target(char **exp, int *num, Buffer *buff);
static int scan_num(char **str, int *num);
static int offset(char **exp, int *num);

int
parse(char *exp, Tokens *tk, Buffer *buff)
{
	tk->start = tk->end = tk->target = INT_MAX;
	tk->cmd = tk->suffix = tk->mark = '\0';
	free(tk->arg);
	tk->arg = NULL;

	/* range */
	if (!parse_range(&exp, tk, buff)) {
		error("Invalid address");
		return 0;
	}

	if (tk->start > tk->end) {
		tk->start = tk->start ^ tk->end;
		tk->end = tk->start ^ tk->end;
		tk->start = tk->start ^ tk->end;
	}

	/* command */
	tk->cmd = *exp ? *exp : 'p';

	if (strchr("qQPhHfeE", tk->cmd) && tk->start != INT_MAX) {
		error("Unexpected address");
		return 0;
	}

	/* default value */
	if (tk->start == INT_MAX)
		tk->start = tk->end = buff->curr_line;

	if (tk->end == INT_MAX)
		tk->end = tk->start;

	/* check range */
	int min = !strchr("imtqQPhHfeE", tk->cmd);

	if (tk->start < min || tk->start > buff->last_line) {
		error("Invalid address");
		return 0;
	}

	if (tk->end < min || tk->end > buff->last_line) {
		error("Invalid address");
		return 0;
	}

	/* mark */
	if (tk->cmd == 'k') {
		exp++;
		if (!isalpha(*exp)) {
			error("Expected marker");
			return 0;
		}
		tk->mark = *exp;
		exp++;
	}

	if (!*exp) return 1;
	exp++;

	/* arg */
	if (strchr("eEfgsw/", tk->cmd)) {
		while (isblank(*exp)) exp++;
		if (!*exp)
			return 1;
		tk->arg = estrdup(exp);
		return 1;
	}

	/* target */
	while (isblank(*exp)) exp++;
	if (strchr("mt", tk->cmd)) {
		if (!parse_target(&exp, &tk->target, buff)) {
			error("Invalid address");
			return 0;
		}
		if (tk->target == INT_MAX)
			tk->target = buff->curr_line;
		if (tk->cmd == 'm' && tk->start <= tk->target && tk->end > tk->target) {
			error("Invalid destination");
			return 0;
		}
		if (tk->target < 0 || tk->target > buff->last_line) {
			error("Invalid address");
			return 0;
		}
	}

	/* suffix */
	while (isblank(*exp)) exp++;
	if (isalpha(*exp)) {
		tk->suffix = *exp;
		exp++;
		if (strchr("eEfqQrw", tk->cmd)) {
			error("Unexpected command suffix");
			return 0;
		}
		if (!strchr("lnp", tk->suffix)) {
			error("Invalid suffix");
			return 0;
		}
	}


	/* check if everything is parsed */
	while (isblank(*exp)) exp++;
	if (*exp) {
		error("Invalid suffix");
		return 0;
	}
	return 1;
}

int
scan_num(char **str, int *num)
{
	int parsed;
	int filled = sscanf(*str, "%d%n", num, &parsed);
	if (filled > 0) *str += parsed;
	return filled > 0;
}

int
parse_target(char **exp, int *num, Buffer *buff)
{
	while (isblank(**exp)) ++*exp;
	if (!**exp) return 1;

	switch (**exp) {
	case '\'':
		++*exp;
		*num = buff->marks[(int)**exp];
		++*exp;
		break;
	case '.':
		*num = buff->curr_line;
		++*exp;
		break;
	case '$':
		*num = buff->last_line;
		++*exp;
		break;
	case '-':
	case '+':
		if (scan_num(exp, num))
			*num += buff->curr_line;
		else {
			*num = buff->curr_line + (**exp == '-' ? -1 : +1);
			++*exp;
		}
		break;
	default:
		scan_num(exp, num);
		break;
	}

	return offset(exp, num) == 1;
}

int
parse_range(char **exp, Tokens *tk, Buffer *buff)
{
	int *num = tk->start == INT_MAX ? &tk->start : &tk->end;

	while (isblank(**exp)) ++*exp;
	if (!**exp) return 1;

	switch (**exp) {
	case '%':
		if (num == &tk->end)
			return 0;
		tk->start = 1;
		tk->end = buff->last_line;
		++*exp;
		return 1;
	case '\'':
		++*exp;
		*num = buff->marks[(int)**exp];
		++*exp;
		break;
	case '.':
		*num = buff->curr_line;
		++*exp;
		break;
	case '$':
		*num = buff->last_line;
		++*exp;
		break;
	case '-':
	case '+':
		if (scan_num(exp, num))
			*num += buff->curr_line;
		else {
			*num = buff->curr_line + (**exp == '-' ? -1 : +1);
			++*exp;
		}
		break;
	default:
		scan_num(exp, num);
		break;
	}

	int ret = offset(exp, num);
	if (ret < 2)
		return ret;

	++*exp;

	if (tk->end == INT_MAX)
		tk->end = tk->start;
	else
		tk->start = tk->end;

	/* default value if no range specified (only separator) */
	if (tk->start == INT_MAX) {
		tk->start = ret == 2 ? 1 : buff->curr_line;
		tk->end = buff->last_line;
	}

	return parse_range(exp, tk, buff);
}

int
offset(char **exp, int *num)
{
	while (isblank(**exp)) ++*exp;

	if (!**exp) return 1;
	if (**exp == ',') return 2;
	if (**exp == ';') return 3;

	int tmp;
	switch (**exp) {
	case '+':
		++*exp;
		if (scan_num(exp, &tmp))
			*num += tmp;
		else
			++*num;
		break;
	case '-':
		++*exp;
		if (scan_num(exp, &tmp))
			*num -= tmp;
		else
			--*num;
		break;
	default:
		if (isdigit(**exp)) {
			scan_num(exp, &tmp);
			*num += tmp;
			break;
		} else if (isalpha(**exp))
			return 1;
		else
			return 0;
	}
	return offset(exp, num);
}
