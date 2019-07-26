INTERMEDIATE_DIR?=stage
OUTPUT_DIR?=bin

SRC_DIRS=.
C_FILES=$(wildcard ${dir}/*.c)
C_SOURCES=$(foreach dir,${SRC_DIRS},${C_FILES})
C_OBJECTS=$(patsubst %.c,${INTERMEDIATE_DIR}/%.c.o,${C_SOURCES})
ASM_FILES=$(wildcard ${dir}/*.s)
ASM_SOURCES=$(foreach dir,${SRC_DIRS},${ASM_FILES})
ASM_OBJECTS=$(patsubst %.s,${INTERMEDIATE_DIR}/%.s.o,${ASM_SOURCES})
OBJECTS=${ASM_OBJECTS} ${C_OBJECTS}
INTERMEDIATE_DIRS=$(foreach dir,${SRC_DIRS},${INTERMEDIATE_DIR}/${dir})

CC=gcc
CFLAGS=-m64 -std=gnu99 -O0 -Wall -Wextra -Werror -g -fpic

LD=gcc
LDFLAGS=-shared

AR=ar
ARFLAGS=rcs

AS=as
ASFLAGS=

${OUTPUT_DIR}/libelf-hook.so: ${OBJECTS} ${OUTPUT_DIR}
	${LD} ${LDFLAGS} -o $@ ${OBJECTS} -ldl

${OUTPUT_DIR}:
	mkdir $@

${INTERMEDIATE_DIR}:
	mkdir $@

${OBJECTS}: | ${INTERMEDIATE_DIRS}

${INTERMEDIATE_DIRS}:
	@mkdir -p $@

${INTERMEDIATE_DIR}/%.c.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

${INTERMEDIATE_DIR}/%.s.o: %.s
	${AS} ${ASFLAGS} $< -o $@

all: ${OUTPUT_DIR}/libelf-hook.so

clean:
	rm -f ${OUTPUT_DIR}/libelf-hook.so
	rm -f ${OBJECTS}

.PHONY: all clean