VERSION := 0.1

INC :=
LIBS :=

WARN := -Wall -Wextra -Wpedantic
DEBUG := -g -DDEBUG
RELEASE := -O3 -flto -DNODEBUG

CFLAGS := ${INC} ${WARN} ${DEBUG} #${RELEASE}
LDFLAGS := ${LIBS}
