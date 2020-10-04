include config.mk

SRC := ed.c util.c buffer.c parser.c
OBJ = ${SRC:.c=.o}

ed: ${OBJ}
	${CC} ${LDFLAGS} -o $@ $^

obj := ${filter-out ed.o,${OBJ}}
ed.o: arg.h ${obj}
${obj}: %.o: %.h util.h

clean:
	${RM} ${OBJ} ed

.PHONY: clean
